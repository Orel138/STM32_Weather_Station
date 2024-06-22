/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "string.h"
#include "stdio.h"

/* Project Specific (Wi-Fi Driver for ESP8266)*/
#include "esp8266.h"
#include "esp8266_io.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
extern RNG_HandleTypeDef hrng;
extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define xConsoleHandle huart1
#define xESP8266Handle huart5
#define xLORAHandle huart6
#define xLRWANHandle huart6
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_USART1_UART_Init(void);
void MX_RNG_Init(void);
void MX_UART5_Init(void);

/* USER CODE BEGIN EFP */
int main_app( void );
int hw_init( void );
void vDoSystemReset( void );
static inline void vPetWatchdog( void )
{
	(void) HAL_IWDG_Refresh( &hiwdg );
}
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ESP8266_TX_Pin GPIO_PIN_12
#define ESP8266_TX_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOJ
#define ESP8266_RST_Pin GPIO_PIN_14
#define ESP8266_RST_GPIO_Port GPIOJ
#define ESP8266_RX_Pin GPIO_PIN_2
#define ESP8266_RX_GPIO_Port GPIOD
#define VCP_RX_Pin GPIO_PIN_10
#define VCP_RX_GPIO_Port GPIOA
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define VCP_TX_Pin GPIO_PIN_9
#define VCP_TX_GPIO_Port GPIOA
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define CEC_CLK_Pin GPIO_PIN_8
#define CEC_CLK_GPIO_Port GPIOA
#define OSC_25M_Pin GPIO_PIN_0
#define OSC_25M_GPIO_Port GPIOH
#define LPS22HB_INT1_Pin GPIO_PIN_8
#define LPS22HB_INT1_GPIO_Port GPIOC
#define LPS22HB_INT1_EXTI_IRQn EXTI9_5_IRQn
#define DHT11_DATA_Pin GPIO_PIN_10
#define DHT11_DATA_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOJ
#define USER_BUTTON_Pin GPIO_PIN_0
#define USER_BUTTON_GPIO_Port GPIOA
#define USER_BUTTON_EXTI_IRQn EXTI0_IRQn

/* USER CODE BEGIN Private defines */
extern void *LPS22HB_P_0_handle;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
