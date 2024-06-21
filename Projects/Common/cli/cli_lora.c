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


/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Project Specific */
#include "cli.h"
#include "cli_prv.h"
#include "logging.h"

/* Project Specific */
#include "wm_sg_sm_42.h"
#include "wm_sg_sm_42_io.h"

/* Standard Lib */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Local static functions */
static void vCommand_LoRa( ConsoleIO_t * pxCIO,
                           uint32_t ulArgc,
                           char * ppcArgv[] );

const CLI_Command_Definition_t xCommandDef_lora =
{
    .pcCommand            = "lora",
    .pcHelpString         =
        "lora:\r\n"
        "    Perform operations on the USI WM-SG-SM-42 LoRa module.\r\n"
        "    Usage:\r\n"
        "    lora <verb> <object> <args>\r\n"
        "        Valid verbs are { connect, disconnect, status, scan }\r\n"
        "    lora connect <ssid> <password>\r\n"
        "        Connect to a Wi-Fi SSID\r\n\n"
        "    lora disconnect\r\n"
        "        Disconnect from the current SSID\r\n\n"
        "    lora ip\r\n"
        "        Get the current ip address\r\n\n"
        "    lora scan\r\n"
        "        Scan for available Wi-Fi networks\r\n\n"
        "    lora ntp\r\n"
        "        Get the current time from an NTP server\r\n\n",
    .pxCommandInterpreter = vCommand_LoRa
};

#define VERB_ARG_INDEX       1
#define OBJECT_TYPE_INDEX    2

static void vCommand_LoRa( ConsoleIO_t * pxCIO,
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
            	xSuccess = pdTRUE;
            }
            else
            {
                pxCIO->print( "Error: SSID and password required for connect command.\r\n" );
                xSuccess = pdFALSE;
            }
        }
        else if( 0 == strcmp( "disconnect", pcVerb ) )
        {
        	xSuccess = pdTRUE;
        }
        else if( 0 == strcmp( "version", pcVerb ) )
        {
        	xSuccess = pdTRUE;
        }
        else if( 0 == strcmp( "scan", pcVerb ) )
        {
            pxCIO->print( "Failed to scan for Wi-Fi networks.\r\n" );
            xSuccess = pdFALSE;

        }
    }

    if( xSuccess == pdFALSE )
    {
        pxCIO->print( xCommandDef_lora.pcHelpString );
    }
}
