/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com. Based extensively on ST's code  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <stm32f4xx.h>
#include "simplesdio.h"
#include "logf.h"

/**
 * @brief  SDIO Static flags, TimeOut, FIFO Address
 */
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00010000)

/**
 * @brief  Mask for errors Card Status R1 (OCR Register)
 */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/**
 * @brief  Masks for R6 Response
 */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/**
 * @brief  Command Class Supported
 */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/**
 * @brief  Following commands are SD Card Specific commands.
 *         SDIO_APP_CMD should be sent before sending these commands.
 */
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)

/*--------------------------------------------------------------------------*/

/*
 * Card state global variables.
 */
static uint32_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;

/*--------------------------------------------------------------------------*/

static SD_Error cmdError (void);
static SD_Error cmdResp7Error (void);
static SD_Error cmdResp1Error (uint8_t cmd);
static SD_Error cmdResp3Error (void);
static SD_Error cmdResp2Error (void);
static SD_Error cmdResp6Error (uint8_t cmd, uint16_t *prca);

/*--------------------------------------------------------------------------*/

/**
 * @brief  Enquires cards about their operating voltage and configures clock controls.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error sdPowerOn (void)
{
        __IO SD_Error
        errorstatus = SD_OK;
        uint32_t response = 0, count = 0, validvoltage = 0;
        uint32_t SDType = SD_STD_CAPACITY;

        /*!< Power ON Sequence -----------------------------------------------------*/
        /*!< Configure the SDIO peripheral */
        /*!< SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) */
        /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
        /*!< SDIO_CK for initialization should not exceed 400 KHz */
        SDIO_InitTypeDef SDIO_InitStructure;

        SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
        SDIO_Init (&SDIO_InitStructure);

        /*!< Set Power State to ON */
        SDIO_SetPowerState (SDIO_PowerState_ON);

        /*!< Enable SDIO Clock */
        SDIO_ClockCmd (ENABLE);

        /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
        /*!< No CMD response required */
        SDIO_CmdInitTypeDef SDIO_CmdInitStructure;

        SDIO_CmdInitStructure.SDIO_Argument = 0x0;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand (&SDIO_CmdInitStructure);

        errorstatus = cmdError ();

        if (errorstatus != SD_OK) {
                /*!< CMD Response TimeOut (wait for CMDSENT flag) */
                logf ("GO_IDLE_STATE (CMD0) send failed\r\n");
                return (errorstatus);
        }
        else {
                logf ("GO_IDLE_STATE (CMD0) OK\r\n");
        }

        /*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
        /*!< Send CMD8 to verify SD card interface operating condition */
        /*!< Argument: - [31:12]: Reserved (shall be set to '0')
         - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
         - [7:0]: Check Pattern (recommended 0xAA) */
        /*!< CMD Response: R7 */
        SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand (&SDIO_CmdInitStructure);

        errorstatus = cmdResp7Error ();

        if (errorstatus == SD_OK) {
                CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
                SDType = SD_HIGH_CAPACITY;
        }
        else {
                /*!< CMD55 */
                SDIO_CmdInitStructure.SDIO_Argument = 0x00;
                SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
                SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                SDIO_SendCommand (&SDIO_CmdInitStructure);
                errorstatus = cmdResp1Error (SD_CMD_APP_CMD);
        }
        /*!< CMD55 */
        SDIO_CmdInitStructure.SDIO_Argument = 0x00;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand (&SDIO_CmdInitStructure);
        errorstatus = cmdResp1Error (SD_CMD_APP_CMD);

        /*!< If errorstatus is Command TimeOut, it is a MMC card */
        /*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
         or SD card 1.x */
        if (errorstatus == SD_OK) {
                /*!< SD CARD */
                /*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
                while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL)) {

                        /*!< SEND CMD55 APP_CMD with RCA as 0 */
                        SDIO_CmdInitStructure.SDIO_Argument = 0x00;
                        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
                        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                        SDIO_SendCommand (&SDIO_CmdInitStructure);

                        errorstatus = cmdResp1Error (SD_CMD_APP_CMD);

                        if (errorstatus != SD_OK) {
                                return (errorstatus);
                        }
                        SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
                        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
                        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                        SDIO_SendCommand (&SDIO_CmdInitStructure);

                        errorstatus = cmdResp3Error ();
                        if (errorstatus != SD_OK) {
                                return (errorstatus);
                        }

                        response = SDIO_GetResponse (SDIO_RESP1);
                        validvoltage = (((response >> 31) == 1) ? 1 : 0);
                        count++;
                }
                if (count >= SD_MAX_VOLT_TRIAL) {
                        errorstatus = SD_INVALID_VOLTRANGE;
                        return (errorstatus);
                }

                if (response &= SD_HIGH_CAPACITY) {
                        CardType = SDIO_HIGH_CAPACITY_SD_CARD;
                }

        }/*!< else MMC Card */

        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for CMD0.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error cmdError (void)
{
        SD_Error errorstatus = SD_OK;
        uint32_t timeout;

        timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

        while ((timeout > 0) && (SDIO_GetFlagStatus (SDIO_FLAG_CMDSENT) == RESET)) {
                timeout--;
        }

        if (timeout == 0) {
                errorstatus = SD_CMD_RSP_TIMEOUT;
                return (errorstatus);
        }

        /*!< Clear all the static flags */
        SDIO_ClearFlag (SDIO_STATIC_FLAGS );

        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R7 response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error cmdResp7Error (void)
{
        SD_Error errorstatus = SD_OK;
        uint32_t status;
        uint32_t timeout = SDIO_CMD0TIMEOUT;

        status = SDIO ->STA;

        while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0)) {
                timeout--;
                status = SDIO ->STA;
        }

        if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT)) {
                /*!< Card is not V2.0 complient or card does not support the set voltage range */
                errorstatus = SD_CMD_RSP_TIMEOUT;
                SDIO_ClearFlag (SDIO_FLAG_CTIMEOUT);
                return (errorstatus);
        }

        if (status & SDIO_FLAG_CMDREND) {
                /*!< Card is SD V2.0 compliant */
                errorstatus = SD_OK;
                SDIO_ClearFlag (SDIO_FLAG_CMDREND);
                return (errorstatus);
        }
        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R1 response.
 * @param  cmd: The sent command index.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error cmdResp1Error (uint8_t cmd)
{
        SD_Error errorstatus = SD_OK;
        uint32_t status;
        uint32_t response_r1;

        status = SDIO ->STA;

        while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))) {
                status = SDIO ->STA;
        }

        if (status & SDIO_FLAG_CTIMEOUT) {
                errorstatus = SD_CMD_RSP_TIMEOUT;
                SDIO_ClearFlag (SDIO_FLAG_CTIMEOUT);
                return (errorstatus);
        }
        else if (status & SDIO_FLAG_CCRCFAIL) {
                errorstatus = SD_CMD_CRC_FAIL;
                SDIO_ClearFlag (SDIO_FLAG_CCRCFAIL);
                return (errorstatus);
        }

        /*!< Check response received is of desired command */
        if (SDIO_GetCommandResponse () != cmd) {
                errorstatus = SD_ILLEGAL_CMD;
                return (errorstatus);
        }

        /*!< Clear all the static flags */
        SDIO_ClearFlag (SDIO_STATIC_FLAGS );

        /*!< We have received response, retrieve it for analysis  */
        response_r1 = SDIO_GetResponse (SDIO_RESP1);

        if ((response_r1 & SD_OCR_ERRORBITS )== SD_ALLZERO) {
                return (errorstatus);
        }

        if (response_r1 & SD_OCR_ADDR_OUT_OF_RANGE ) {
                return (SD_ADDR_OUT_OF_RANGE);
        }

        if (response_r1 & SD_OCR_ADDR_MISALIGNED ) {
                return (SD_ADDR_MISALIGNED);
        }

        if (response_r1 & SD_OCR_BLOCK_LEN_ERR ) {
                return (SD_BLOCK_LEN_ERR);
        }

        if (response_r1 & SD_OCR_ERASE_SEQ_ERR ) {
                return (SD_ERASE_SEQ_ERR);
        }

        if (response_r1 & SD_OCR_BAD_ERASE_PARAM ) {
                return (SD_BAD_ERASE_PARAM);
        }

        if (response_r1 & SD_OCR_WRITE_PROT_VIOLATION ) {
                return (SD_WRITE_PROT_VIOLATION);
        }

        if (response_r1 & SD_OCR_LOCK_UNLOCK_FAILED ) {
                return (SD_LOCK_UNLOCK_FAILED);
        }

        if (response_r1 & SD_OCR_COM_CRC_FAILED ) {
                return (SD_COM_CRC_FAILED);
        }

        if (response_r1 & SD_OCR_ILLEGAL_CMD ) {
                return (SD_ILLEGAL_CMD);
        }

        if (response_r1 & SD_OCR_CARD_ECC_FAILED ) {
                return (SD_CARD_ECC_FAILED);
        }

        if (response_r1 & SD_OCR_CC_ERROR ) {
                return (SD_CC_ERROR);
        }

        if (response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR ) {
                return (SD_GENERAL_UNKNOWN_ERROR);
        }

        if (response_r1 & SD_OCR_STREAM_READ_UNDERRUN ) {
                return (SD_STREAM_READ_UNDERRUN);
        }

        if (response_r1 & SD_OCR_STREAM_WRITE_OVERRUN ) {
                return (SD_STREAM_WRITE_OVERRUN);
        }

        if (response_r1 & SD_OCR_CID_CSD_OVERWRIETE ) {
                return (SD_CID_CSD_OVERWRITE);
        }

        if (response_r1 & SD_OCR_WP_ERASE_SKIP ) {
                return (SD_WP_ERASE_SKIP);
        }

        if (response_r1 & SD_OCR_CARD_ECC_DISABLED ) {
                return (SD_CARD_ECC_DISABLED);
        }

        if (response_r1 & SD_OCR_ERASE_RESET ) {
                return (SD_ERASE_RESET);
        }

        if (response_r1 & SD_OCR_AKE_SEQ_ERROR ) {
                return (SD_AKE_SEQ_ERROR);
        }
        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R3 (OCR) response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error cmdResp3Error (void)
{
        SD_Error errorstatus = SD_OK;
        uint32_t status;

        status = SDIO ->STA;

        while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))) {
                status = SDIO ->STA;
        }

        if (status & SDIO_FLAG_CTIMEOUT) {
                errorstatus = SD_CMD_RSP_TIMEOUT;
                SDIO_ClearFlag (SDIO_FLAG_CTIMEOUT);
                return (errorstatus);
        }
        /*!< Clear all the static flags */
        SDIO_ClearFlag (SDIO_STATIC_FLAGS );
        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R2 (CID or CSD) response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error cmdResp2Error (void)
{
        SD_Error errorstatus = SD_OK;
        uint32_t status;

        status = SDIO ->STA;

        while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND))) {
                status = SDIO ->STA;
        }

        if (status & SDIO_FLAG_CTIMEOUT) {
                errorstatus = SD_CMD_RSP_TIMEOUT;
                SDIO_ClearFlag (SDIO_FLAG_CTIMEOUT);
                return (errorstatus);
        }
        else if (status & SDIO_FLAG_CCRCFAIL) {
                errorstatus = SD_CMD_CRC_FAIL;
                SDIO_ClearFlag (SDIO_FLAG_CCRCFAIL);
                return (errorstatus);
        }

        /*!< Clear all the static flags */
        SDIO_ClearFlag (SDIO_STATIC_FLAGS );

        return (errorstatus);
}

/**
 * @brief  Checks for error conditions for R6 (RCA) response.
 * @param  cmd: The sent command index.
 * @param  prca: pointer to the variable that will contain the SD card relative
 *         address RCA.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error cmdResp6Error (uint8_t cmd, uint16_t *prca)
{
        SD_Error errorstatus = SD_OK;
        uint32_t status;
        uint32_t response_r1;

        status = SDIO ->STA;

        while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND))) {
                status = SDIO ->STA;
        }

        if (status & SDIO_FLAG_CTIMEOUT) {
                errorstatus = SD_CMD_RSP_TIMEOUT;
                SDIO_ClearFlag (SDIO_FLAG_CTIMEOUT);
                return (errorstatus);
        }
        else if (status & SDIO_FLAG_CCRCFAIL) {
                errorstatus = SD_CMD_CRC_FAIL;
                SDIO_ClearFlag (SDIO_FLAG_CCRCFAIL);
                return (errorstatus);
        }

        /*!< Check response received is of desired command */
        if (SDIO_GetCommandResponse () != cmd) {
                errorstatus = SD_ILLEGAL_CMD;
                return (errorstatus);
        }

        /*!< Clear all the static flags */
        SDIO_ClearFlag (SDIO_STATIC_FLAGS );

        /*!< We have received response, retrieve it.  */
        response_r1 = SDIO_GetResponse (SDIO_RESP1);

        if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED ))) {
                *prca = (uint16_t) (response_r1 >> 16);
                return (errorstatus);
        }

        if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR ) {
                return (SD_GENERAL_UNKNOWN_ERROR);
        }

        if (response_r1 & SD_R6_ILLEGAL_CMD ) {
                return (SD_ILLEGAL_CMD);
        }

        if (response_r1 & SD_R6_COM_CRC_FAILED ) {
                return (SD_COM_CRC_FAILED);
        }

        return (errorstatus);
}
