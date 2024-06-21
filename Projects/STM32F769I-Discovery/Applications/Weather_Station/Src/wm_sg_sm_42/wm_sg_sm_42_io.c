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
#include "main.h"
#include "wm_sg_sm_42_io.h"

/* Private define ------------------------------------------------------------*/
#define RING_BUFFER_SIZE                         (1024 * 2)

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  data[RING_BUFFER_SIZE];
  uint16_t tail;
  uint16_t head;
}RingBufferLoRa_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RingBufferLoRa_t LoraRxBuffer;
UART_HandleTypeDef LoraUartHandle;

/* Private function prototypes -----------------------------------------------*/
static void LORA_Handler(void);

/* Private functions ---------------------------------------------------------*/

int8_t LORA_IO_Init(void)
{
    LoraRxBuffer.head = 0;
    LoraRxBuffer.tail = 0;

    HAL_UART_Receive_IT(&xLORAHandle, (uint8_t *)&LoraRxBuffer.data[LoraRxBuffer.tail], 1);

    return 0;
}

void LORA_IO_DeInit(void)
{
    HAL_UART_DeInit(&xLORAHandle);
}

int8_t LORA_IO_Send(uint8_t* pData, uint32_t Length)
{
    if (HAL_UART_Transmit(&xLORAHandle, (uint8_t*)pData, Length, DEFAULT_TIME_OUT) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

int32_t LORA_IO_Receive(uint8_t* Buffer, uint32_t Length)
{
    uint32_t ReadData = 0;

    while (Length--)
    {
        uint32_t tickStart = HAL_GetTick();
        do
        {
            if (LoraRxBuffer.head != LoraRxBuffer.tail)
            {
                *Buffer++ = LoraRxBuffer.data[LoraRxBuffer.head++];
                ReadData++;

                if (LoraRxBuffer.head >= RING_BUFFER_SIZE)
                {
                    LoraRxBuffer.head = 0;
                }
                break;
            }
        } while ((HAL_GetTick() - tickStart) < DEFAULT_TIME_OUT);
    }

    return ReadData;
}

void HAL_UART_RxCpltCallback_LORAHandler(UART_HandleTypeDef *UartHandle)
{
    if (++LoraRxBuffer.tail >= RING_BUFFER_SIZE)
    {
        LoraRxBuffer.tail = 0;
    }

    HAL_UART_Receive_IT(UartHandle, (uint8_t *)&LoraRxBuffer.data[LoraRxBuffer.tail], 1);
}

void HAL_UART_ErrorCallback_LORAHandler(UART_HandleTypeDef *UartHandle)
{
    LORA_Handler();
}

static void LORA_Handler(void)
{
    HAL_UART_DeInit(&xLORAHandle);

    while(1)
    {
    }
}
