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

#ifndef __ESP8266_REST_API_H
#define __ESP8266_REST_API_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <time.h>

#include "esp8266.h"
#include "esp8266_io.h"
#include "esp8266_user_config.h"

/* Private define ------------------------------------------------------------*/
/* Maximum size of response */
#define DATA_MAX_SIZE   (50 * 1024)

#define MAX_NUM_TRIAL 10

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define ARRAY_SIZE(array)     ((int)(sizeof(array) / sizeof((array)[0])))

/* Exported functions ------------------------------------------------------- */
ESP8266_StatusTypeDef get_weather_data(char *city, char *country, char *response, size_t *response_size);
ESP8266_StatusTypeDef send_telemetry_data(float temperature, float humidity, char *response, size_t response_size);
 
#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_REST_API_H */
