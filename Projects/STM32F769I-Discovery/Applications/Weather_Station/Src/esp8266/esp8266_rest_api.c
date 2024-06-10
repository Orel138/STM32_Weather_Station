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

#include "esp8266_rest_api.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Project Specific */
#include "cli.h"
#include "cli_prv.h"
#include "logging.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t HtmlData[DATA_MAX_SIZE];
static uint8_t HtmlRequest[256];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

// Function to get weather data from OpenWeatherMap
ESP8266_StatusTypeDef get_weather_data(char *city, char *country, char *response, size_t *response_size)
{
	uint8_t *Data;
    uint32_t DataSize = 0;
	uint32_t Trial = 0;

	ESP8266_ConnectionInfoTypeDef ConnectionInfo;
	ESP8266_StatusTypeDef Result = ESP8266_OK;

	char HtmlRequest[256];

	/* Initialize the HTML request field to 0 */
	memset(HtmlRequest, '\0', 128);

	/*
	Send an HTML GET request to get weather, the request is as follows
    ====================================
	GET /data/2.5/weather?q=<city>,<country>&appid=<OPENWEATHERMAP_API_KEY>&units=metric HTTP/1.1
	Host: api.openweathermap.org
	User-Agent: <esp8266/stm32_for_example>
	Accept: * / *  <= no space here
	Connection: keep-alive
    ====================================
    the request MUST end with a "\r\n\r\n" otherwise it won't be processed correctly.
    */

    sprintf((char *)HtmlRequest, "GET /data/2.5/weather?q=%s,%s&appid=%s&units=metric HTTP/1.1\r\n"
           	                   	 "Host: %s\r\n"
		   	                   	 "User-Agent: esp8266/stm32\r\n"
		   	                   	 "Accept: */*\r\n"
		    	                 "Connection: keep-alive\r\n\r\n",
								 city, country, OPENWEATHERMAP_API_KEY, OPENWEATHERMAP_HOST);

    /* Initialize Connection info structure */
    memset(&ConnectionInfo, '\0', sizeof (ESP8266_ConnectionInfoTypeDef));

	ConnectionInfo.connectionType = ESP8266_TCP_CONNECTION;
	ConnectionInfo.ipAddress = (uint8_t *)OPENWEATHERMAP_HOST;
	ConnectionInfo.isServer = ESP8266_FALSE;
	ConnectionInfo.port = OPENWEATHERMAP_PORT;

    /* Wait for communication establishment */
    while (ESP8266_EstablishConnection(&ConnectionInfo) != ESP8266_OK)
    {
    	if (Trial == MAX_NUM_TRIAL)
    	{
    		break;
    	}
    }

    /* Check if trials number exceeded maximum limit */
    if (Trial == MAX_NUM_TRIAL)
    {
    	/* Leave the access point */
    	ESP8266_QuitAccessPoint();

    	/* Deinitialize the WiFi module */
    	ESP8266_DeInit();

    	/* Call the error Handler */
    	Error_Handler();
    }

    /* Reset the HTML data field to 0 */
    memset(HtmlData, '\0', ARRAY_SIZE(HtmlData));

    Result = ESP8266_SendData(HtmlRequest, strlen((char *)HtmlRequest));

    /* In case of error, quit the Access point */
    if (Result != ESP8266_OK)
    {
    	/* Deinitialize the WiFi module */
    	ESP8266_DeInit();

    	/* Call the error Handler */
    	Error_Handler();
    }

	Result = ESP8266_ReceiveData(HtmlData, DATA_MAX_SIZE, &DataSize);

	/* If data reception failed */
   	if (Result != ESP8266_OK)
   	{
   		/* Leave the access point */
		ESP8266_QuitAccessPoint();

		/* Deinitialize the WiFi module */
		ESP8266_DeInit();

		/* Call the error Handler */
		Error_Handler();
	}

   	/* If data reception passed */
   	if(strstr((char *)HtmlData, OPENWEATHERMAP_HTML_SUCCESS_STRING) == NULL)
   	{
   		/* Leave the access point */
		ESP8266_QuitAccessPoint();

		/* Deinitialize the WiFi module */
		ESP8266_DeInit();

		/* Call the error Handler */
		Error_Handler();
   	}

   	/* The data is located after the HTML HEADER
   	 * the end of header is marked with the string "\r\n\r\n" */

   	LogInfo(HtmlData);

   	/* Find the start of the JSON data by locating the "\r\n\r\n" sequence */
   	char *json_start = strstr((char *)HtmlData, "\r\n\r\n");

   	/* Check if the data is null */
	if (json_start != NULL)
	{
		/* Increment the pointer by 4 to skip the "\r\n\r\n" sequence */
		json_start += 4;

		LogInfo(json_start);

	}
	else
	{
		/* Leave the access point */
		ESP8266_QuitAccessPoint();

		/* Deinitialize the WiFi module */
		ESP8266_DeInit();

		/* Call the error Handler */
		Error_Handler();
	}

	/* Stop WiFi module */
	ESP8266_DeInit();
}

// Function to put weather data on OpenSenseMap
ESP8266_StatusTypeDef send_telemetry_data(float temperature, float humidity, char *response, size_t response_size)
{
	uint8_t *Data;
    uint32_t DataSize = 0;
	uint32_t Trial = 0;

	ESP8266_ConnectionInfoTypeDef ConnectionInfo;
	ESP8266_StatusTypeDef Result = ESP8266_OK;

	char HtmlRequest[256];

	/* Initialize the HTML request field to 0 */
	memset(HtmlRequest, '\0', 128);

	/*
	Send an HTML GET request to get weather, the request is as follows
    ====================================
	POST /boxes/<boxe_id>/<sensor_id> HTTP/1.1
	Host: <OPENSENSEMAP_HOST>
	Content-Type: application/json
	Content-Length: 13

	{"value":"25.0"}
    ====================================
    the request MUST end with a "\r\n\r\n" otherwise it won't be processed correctly.
    */

	sprintf((char *)HtmlRequest, "POST /boxes/%s/%s HTTP/1.1\r\n"
			                     "Host: %s\r\n"
			                     "Content-Type: application/json\r\n"
			                     "Content-Length: 13\r\n"
			                     "\r\n"
			                     "{\"value\":\"25.0\"}\r\n\r\n",
								 OPENSENSEMAP_BOX, OPENSENSEMAP_SENSOR1, OPENSENSEMAP_HOST);

    /* Initialize Connection info structure */
    memset(&ConnectionInfo, '\0', sizeof (ESP8266_ConnectionInfoTypeDef));

	ConnectionInfo.connectionType = ESP8266_TCP_CONNECTION;
	ConnectionInfo.ipAddress = (uint8_t *)OPENSENSEMAP_HOST;
	ConnectionInfo.isServer = ESP8266_FALSE;
	ConnectionInfo.port = OPENSENSEMAP_PORT;

    /* Wait for communication establishment */
    while (ESP8266_EstablishConnection(&ConnectionInfo) != ESP8266_OK)
    {
    	if (Trial == MAX_NUM_TRIAL)
    	{
    		break;
    	}
    }

    /* Check if trials number exceeded maximum limit */
    if (Trial == MAX_NUM_TRIAL)
    {
    	/* Leave the access point */
    	ESP8266_QuitAccessPoint();

    	/* Deinitialize the WiFi module */
    	ESP8266_DeInit();

    	/* Call the error Handler */
    	Error_Handler();
    }

    /* Reset the HTML data field to 0 */
    memset(HtmlData, '\0', ARRAY_SIZE(HtmlData));

    Result = ESP8266_SendData(HtmlRequest, strlen((char *)HtmlRequest));

    /* In case of error, quit the Access point */
    if (Result != ESP8266_OK)
    {
    	/* Deinitialize the WiFi module */
    	ESP8266_DeInit();

    	/* Call the error Handler */
    	Error_Handler();
    }

	Result = ESP8266_ReceiveData(HtmlData, DATA_MAX_SIZE, &DataSize);

	/* If data reception failed */
   	if (Result != ESP8266_OK)
   	{
   		/* Leave the access point */
		ESP8266_QuitAccessPoint();

		/* Deinitialize the WiFi module */
		ESP8266_DeInit();

		/* Call the error Handler */
		Error_Handler();
	}

   	/* The data is located after the HTML HEADER
   	 * the end of header is marked with the string "\r\n\r\n" */

   	LogInfo(HtmlData);

	/* Stop WiFi module */
	ESP8266_DeInit();
}
