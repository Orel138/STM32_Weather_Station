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
/* Project Specific */
#include "dht11.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define DHT11_GPIO_PORT DHT11_DATA_GPIO_Port
#define DHT11_GPIO_PIN DHT11_DATA_Pin

/* Private variables ---------------------------------------------------------*/
uint8_t data[5];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

// function to read data from DHT11
void DHT11_ReadData(float *temperature, float *humidity)
{
    uint8_t i;
    uint8_t data[5];

    // send start signal
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(18)); // Replace HAL_Delay(18)
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);

    // wait for response
    vTaskDelay(pdMS_TO_TICKS(40)); // Replace HAL_Delay(40)

    // initialize data array
    memset(data, 0, sizeof(data));

    // read 40 bits of data
    for(i = 0; i < 40; i++)
    {
        // wait for low pulse
        while(!HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN));

        // wait for high pulse
        uint32_t t = 0;
        while(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN))
        {
            t++;
            vTaskDelay(pdMS_TO_TICKS(1)); // Replace HAL_Delay(1)
        }

        // store bit value in data array
        if(t > 30)
            data[i / 8] |= (1 << (7 - (i % 8)));
    }

    // verify checksum
    if(data[4] == (data[0] + data[1] + data[2] + data[3]))
    {
        // convert temperature and humidity values
        *humidity = (data[0] << 8 | data[1]) / 10.0;
        *temperature = ((data[2] & 0x7F) << 8 | data[3]) / 10.0;
        if (data[2] & 0x80) *temperature *= -1;
    }
}

// Create a FreeRTOS task to read data from DHT11
void vTaskDHT11(void *pvParameters)
{
    float temperature = 0.0;
    float humidity = 0.0;

    while(1)
    {
        DHT11_ReadData(&temperature, &humidity);
        // Use temperature and humidity values as needed
        // For example, send them to a queue or log them
        LogInfo("Temperature: %.1fC\r\n", temperature);
        LogInfo("Humidity: %.1f%%\r\n", humidity);

        // Delay before the next reading
        vTaskDelay(pdMS_TO_TICKS(2000)); // Adjust the delay as needed

    }
}
