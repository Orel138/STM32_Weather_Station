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

#ifndef __WM_SG_SM_42_H
#define __WM_SG_SM_42_H

#ifdef __cplusplus
 extern "C" {
#endif
 
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f7xx_hal.h"

/* Private define ------------------------------------------------------------*/
#define MAX_BUFFER_SIZE         1500
#define MAX_AT_CMD_SIZE         256
#define AT_OK_STRING            "OK\r\n"
#define AT_ERROR_STRING         "ERROR\r\n"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
LORA_FALSE         = 0,
LORA_TRUE          = 1
} LORA_Boolean;

typedef enum
{
LORA_OK                            = 0,
LORA_ERROR                         = 1,
LORA_BUSY                          = 2,
LORA_ALREADY_CONNECTED             = 3,
LORA_CONNECTION_CLOSED             = 4,
LORA_TIMEOUT                       = 5,
LORA_IO_ERROR                      = 6,
} LORA_StatusTypeDef;

typedef struct
{
uint8_t*  devAddr;
uint8_t*  appSKey;
uint8_t*  nwkSKey;
} LORA_JoinInfoTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
LORA_StatusTypeDef LORA_Init(void);
LORA_StatusTypeDef LORA_DeInit(void);
LORA_StatusTypeDef LORA_JoinNetwork(uint8_t* devAddr, uint8_t* appSKey, uint8_t* nwkSKey);
LORA_StatusTypeDef LORA_SendData(uint8_t* pData, uint32_t Length);
LORA_StatusTypeDef LORA_ReceiveData(uint8_t* pData, uint32_t Length, uint32_t* retLength);
 
#ifdef __cplusplus
}
#endif

#endif /* __WM_SG_SM_42_H */
