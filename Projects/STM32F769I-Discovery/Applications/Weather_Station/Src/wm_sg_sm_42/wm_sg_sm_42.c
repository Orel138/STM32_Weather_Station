/*
 * Copyright (c) 2024 Orel138
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ------------------------------------------------------------------*/
#include "wm_sg_sm_42.h"
#include "wm_sg_sm_42_io.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t AtCmd[MAX_AT_CMD_SIZE];
uint8_t RxBuffer_LORA[MAX_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static LORA_StatusTypeDef runAtCmd(uint8_t* cmd, uint32_t Length, const uint8_t* Token);
static LORA_StatusTypeDef getData(uint8_t* Buffer, uint32_t Length, uint32_t* RetLength);

/* Private functions ---------------------------------------------------------*/
LORA_StatusTypeDef LORA_Init(void)
{
    LORA_StatusTypeDef Ret;

    if (LORA_IO_Init() < 0)
    {
        return LORA_ERROR;
    }

    // Disable Echo mode
//    memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
//    sprintf((char *)AtCmd, "ATE=0%c%c", '\r', '\n');
//    Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

    	memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
        sprintf((char *)AtCmd, "ATI%c%c", '\r', '\n');
        Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

    if (Ret != LORA_OK)
    {
        return LORA_ERROR;
    }

    // Set LoRa mode
//    memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
//    sprintf((char *)AtCmd, "AT+MODE=LWABP%c%c", '\r', '\n');
//    Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

    return Ret;
}

LORA_StatusTypeDef LORA_DeInit(void)
{
    LORA_StatusTypeDef Ret;
    sprintf((char *)AtCmd, "AT+RESET%c%c", '\r', '\n');
    Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);
    LORA_IO_DeInit();
    return Ret;
}

LORA_StatusTypeDef LORA_JoinNetwork(uint8_t* DevAddr, uint8_t* AppSKey, uint8_t* NwkSKey)
{
    LORA_StatusTypeDef Ret;

    // Set Device Address
    memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
    sprintf((char *)AtCmd, "AT+DEVADDR=%s%c%c", DevAddr, '\r', '\n');
    Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

    // Set Application Session Key
    memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
    sprintf((char *)AtCmd, "AT+APPSKEY=%s%c%c", AppSKey, '\r', '\n');
    Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

    // Set Network Session Key
    memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
    sprintf((char *)AtCmd, "AT+NWKSKEY=%s%c%c", NwkSKey, '\r', '\n');
    Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

    return Ret;
}

LORA_StatusTypeDef LORA_SendData(uint8_t* Buffer, uint32_t Length)
{
    LORA_StatusTypeDef Ret = LORA_OK;

    if (Buffer != NULL)
    {
        uint32_t tickStart;

        // Construct the AT+CIPSEND command
        memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
        sprintf((char *)AtCmd, "AT+SEND=%lu%c%c", Length, '\r', '\n');

        // The AT+SEND command doesn't have a return command until the data is actually sent
        Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t*)AT_OK_STRING);

        // Return Error
        if (Ret != LORA_OK)
        {
            return LORA_ERROR;
        }

        // Wait before sending data
        tickStart = HAL_GetTick();
        while (HAL_GetTick() - tickStart < 500)
        {
        }

        // Send the data
        Ret = runAtCmd(Buffer, Length, (uint8_t*)AT_OK_STRING);
    }

    return Ret;
}

LORA_StatusTypeDef LORA_ReceiveData(uint8_t* pData, uint32_t Length, uint32_t* RetLength)
{
    return getData(pData, Length, RetLength);
}

static LORA_StatusTypeDef runAtCmd(uint8_t* cmd, uint32_t Length, const uint8_t* Token)
{
    uint32_t idx = 0;
    uint8_t RxChar;

    memset(RxBuffer_LORA, '\0', MAX_BUFFER_SIZE);

    if (LORA_IO_Send(cmd, Length) < 0)
    {
        return LORA_ERROR;
    }

    while (1)
    {
        if (LORA_IO_Receive(&RxChar, 1) != 0)
        {
            RxBuffer_LORA[idx++] = RxChar;
        }
        else
        {
            break;
        }

        if (idx == MAX_BUFFER_SIZE)
        {
            break;
        }

        if (strstr((char *)RxBuffer_LORA, (char *)Token) != NULL)
        {
            return LORA_OK;
        }

        if (strstr((char *)RxBuffer_LORA, AT_ERROR_STRING) != NULL)
        {
            return LORA_ERROR;
        }
    }

    return LORA_ERROR;
}

static LORA_StatusTypeDef getData(uint8_t* Buffer, uint32_t Length, uint32_t* RetLength)
{
    uint8_t RxChar;
    uint32_t idx = 0;

    // Reset the reception data length
    *RetLength = 0;

    // Reset the reception buffer
    memset(RxBuffer_LORA, '\0', MAX_BUFFER_SIZE);

    while (1)
    {
        if (LORA_IO_Receive(&RxChar, 1) != 0)
        {
            Buffer[idx++] = RxChar;
            (*RetLength)++;

            // Check if the buffer is full
            if (idx == Length)
            {
                break;
            }
        }
        else
        {
            break;
        }

        // Check for errors in the received data
        if (strstr((char *)RxBuffer_LORA, AT_ERROR_STRING) != NULL)
        {
            return LORA_ERROR;
        }
    }

    return LORA_OK;
}
