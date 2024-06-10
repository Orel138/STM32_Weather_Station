/*
 * FreeRTOS STM32 Reference Integration
 * Copyright (C) 2020-2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Project Specific */
#include "cli.h"
#include "cli_prv.h"
#include "logging.h"

/* Project Specific (Wi-Fi Driver for ESP8266) */
#include "esp8266.h"
#include "esp8266_io.h"
#include "esp8266_rest_api.h"

/* Standard Lib */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Local static functions */
static void vCommand_WiFi( ConsoleIO_t * pxCIO,
                           uint32_t ulArgc,
                           char * ppcArgv[] );

const CLI_Command_Definition_t xCommandDef_wifi =
{
    .pcCommand            = "wifi",
    .pcHelpString         =
        "wifi:\r\n"
        "    Perform operations on the ESP8266 Wi-Fi module.\r\n"
        "    Usage:\r\n"
        "    wifi <verb> <object> <args>\r\n"
        "        Valid verbs are { connect, disconnect, status, scan }\r\n"
        "    wifi connect <ssid> <password>\r\n"
        "        Connect to a Wi-Fi SSID\r\n\n"
        "    wifi disconnect\r\n"
        "        Disconnect from the current SSID\r\n\n"
        "    wifi ip\r\n"
        "        Get the current ip address\r\n\n"
        "    wifi scan\r\n"
        "        Scan for available Wi-Fi networks\r\n\n"
        "    wifi ntp\r\n"
        "        Get the current time from an NTP server\r\n\n",
    .pxCommandInterpreter = vCommand_WiFi
};

#define VERB_ARG_INDEX       1
#define OBJECT_TYPE_INDEX    2

static void vCommand_WiFi( ConsoleIO_t * pxCIO,
                           uint32_t ulArgc,
                           char * ppcArgv[] )
{
    const char * pcVerb = NULL;
    BaseType_t xSuccess = pdFALSE;

    if( ulArgc > VERB_ARG_INDEX )
    {
        pcVerb = ppcArgv[ VERB_ARG_INDEX ];

        if( 0 == strcmp( "connect", pcVerb ) )
        {
            if( ulArgc > OBJECT_TYPE_INDEX + 1 )
            {
            	char * ssid = strtok(ppcArgv[OBJECT_TYPE_INDEX], "\"");
            	char * password = strtok(ppcArgv[OBJECT_TYPE_INDEX + 1], "\"");

            	ESP8266_StatusTypeDef status = ESP8266_ERROR;
            	uint32_t Trial = 0;

            	while(( status = ESP8266_JoinAccessPoint((uint8_t *)ssid, (uint8_t *)password)) != ESP8266_OK)
            	{
            		pxCIO->print("Retrying to Join Access Point.\r\n");
            		Trial++;

            		if (Trial == MAX_NUM_TRIAL) break;
            	}

                if (status == ESP8266_OK)
                {
                    pxCIO->print( "Connected to Wi-Fi network.\r\n" );
                    xSuccess = pdTRUE;
                }
                else
                {
                    pxCIO->print( "Failed to connect to Wi-Fi network.\r\n" );
                    xSuccess = pdFALSE;
                }
            }
            else
            {
                pxCIO->print( "Error: SSID and password required for connect command.\r\n" );
                xSuccess = pdFALSE;
            }
        }
        else if( 0 == strcmp( "disconnect", pcVerb ) )
        {
            ESP8266_StatusTypeDef status = ESP8266_QuitAccessPoint();

            if (status == ESP8266_OK)
            {
                pxCIO->print( "Disconnected from Wi-Fi network.\r\n" );
                xSuccess = pdTRUE;
            }
            else
            {
                pxCIO->print( "Failed to disconnect from Wi-Fi network.\r\n" );
                xSuccess = pdFALSE;
            }
        }
        else if( 0 == strcmp( "ip", pcVerb ) )
        {
            uint8_t ipAddress[15];
            memset(ipAddress, '\0', sizeof(ipAddress));
            ESP8266_StatusTypeDef status = ESP8266_GetIPAddress(ESP8266_STATION_MODE, ipAddress);

            if (status == ESP8266_OK)
            {
                pxCIO->print( "Current IP Address: " );
                pxCIO->write( (char *)ipAddress, strlen((char *)ipAddress) );
                pxCIO->print( "\r\n" );
                xSuccess = pdTRUE;
            }
            else
            {
                pxCIO->print( "Failed to get IP address.\r\n" );
                xSuccess = pdFALSE;
            }
        }
        else if( 0 == strcmp( "scan", pcVerb ) )
        {
//            // Assuming you have a function to scan and print available networks
//            ESP8266_StatusTypeDef status = ESP8266_ListAccessPoints();
//
//            if (status == ESP8266_OK)
//            {
//                pxCIO->print( "Scan completed.\r\n" );
//                xSuccess = pdTRUE;
//            }
//            else
//            {
                pxCIO->print( "Failed to scan for Wi-Fi networks.\r\n" );
                xSuccess = pdFALSE;
//            }
        }
        else if( 0 == strcmp( "get", pcVerb ) )
		{
        	ESP8266_StatusTypeDef Result = ESP8266_OK;
        	char * response = NULL;
			size_t response_size = 0;

        	Result = get_weather_data("Grenoble","FR", response, response_size);
        	if (Result != ESP8266_OK)
        	{
				/* Call the error Handler */
				Error_Handler();
			}

        	pxCIO->write( (char *)response, response_size);

			xSuccess = pdTRUE;
		}
        else if( 0 == strcmp( "put", pcVerb ) )
		{
			ESP8266_StatusTypeDef Result = ESP8266_OK;
			char * response = NULL;
			size_t response_size = 0;

			Result = send_telemetry_data(0, 0, NULL, NULL);
			if (Result != ESP8266_OK)
			{
				/* Call the error Handler */
				Error_Handler();
			}

			pxCIO->write( (char *)response, response_size);

			xSuccess = pdTRUE;
		}
    }

    if( xSuccess == pdFALSE )
    {
        pxCIO->print( xCommandDef_wifi.pcHelpString );
    }
}
