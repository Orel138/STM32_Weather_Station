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
#include "message_buffer.h"
#include "task.h"

/* Project Specific */
#include "cli.h"
#include "cli_prv.h"
#include "logging.h"

/* Project Specific (Wi-Fi Driver for ESP8266)*/
#include "esp8266.h"
#include "esp8266_io.h"

/* Standard Lib */
#include <string.h>
#include <stdlib.h>

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
        "        Valid verbs are { connect, import, export, list }\r\n"
        "        Valid object types are { key, csr, cert }\r\n"
        "        Arguments should be specified in --<arg_name> <value>\r\n\n"
        "    pki connect <ssid> <password>\r\n"
        "        Connect to a Wi-Fi SSID\r\n\n"
        "    pki disconnect\r\n"
        "        Disconnect from the current SSID\r\n\n",
    .pxCommandInterpreter = vCommand_WiFi
};

/*
 * CLI format:
 * Argc   1    2            3
 * Idx    0    1            2
 *        pki  generate     key
 *        pki  generate     csr
 *        pki  import       cert
 */
static void vCommand_WiFi( ConsoleIO_t * pxCIO,
                          uint32_t ulArgc,
                          char * ppcArgv[] )
{
//    const char * pcVerb = NULL;

    BaseType_t xSuccess = pdFALSE;

//    if( ulArgc > VERB_ARG_INDEX )
//    {
//        pcVerb = ppcArgv[ VERB_ARG_INDEX ];
//
//        if( 0 == strcmp( "connect", pcVerb ) )
//        {
//            if( ulArgc > OBJECT_TYPE_INDEX )
//            {
//                const char * pcObject = ppcArgv[ OBJECT_TYPE_INDEX ];
//
//                if( 0 == strcmp( "key", pcObject ) )
//                {
//                    vSubCommand_GenerateKey( pxCIO, ulArgc, ppcArgv );
//                    xSuccess = pdTRUE;
//                }
//                else
//                {
//                    pxCIO->print( "Error: Invalid object type: '" );
//                    pxCIO->print( pcObject );
//                    pxCIO->print( "' specified for generate command.\r\n" );
//                    xSuccess = pdFALSE;
//                }
//            }
//            else
//            {
//                pxCIO->print( "Error: Not enough arguments to 'pki generate' command.\r\n" );
//                xSuccess = pdFALSE;
//            }
//        }
//        else if( 0 == strcmp( "import", pcVerb ) )
//        {
//            if( ulArgc > OBJECT_TYPE_INDEX )
//            {
//                const char * pcObject = ppcArgv[ OBJECT_TYPE_INDEX ];
//
//                if( 0 == strcmp( "key", pcObject ) )
//                {
//                    vSubCommand_ImportPubKey( pxCIO, ulArgc, ppcArgv );
//                    xSuccess = pdTRUE;
//                }
//                else
//                {
//                    pxCIO->print( "Error: Invalid object type: '" );
//                    pxCIO->print( pcObject );
//                    pxCIO->print( "' specified for import command.\r\n" );
//                    xSuccess = pdFALSE;
//                }
//            }
//        }
//        else if( 0 == strcmp( "export", pcVerb ) )
//        {
//            xSuccess = pdFALSE;
//
//            if( ulArgc > OBJECT_TYPE_INDEX )
//            {
//                const char * pcObject = ppcArgv[ OBJECT_TYPE_INDEX ];
//
//                if( 0 == strcmp( "key", pcObject ) )
//                {
//                    vSubCommand_ExportKey( pxCIO, ulArgc, ppcArgv );
//                    xSuccess = pdTRUE;
//                }
//                else if( 0 == strcmp( "cert", pcObject ) )
//                {
//                    vSubCommand_ExportCertificate( pxCIO, ulArgc, ppcArgv );
//                    xSuccess = pdTRUE;
//                }
//                else
//                {
//                    pxCIO->print( "Error: Invalid object type: '" );
//                    pxCIO->print( pcObject );
//                    pxCIO->print( "' specified for import command.\r\n" );
//                    xSuccess = pdFALSE;
//                }
//            }
//        }
//        else if( 0 == strcmp( "list", pcVerb ) )
//        {
//            xSuccess = pdFALSE;
//        }
//    }

    if( xSuccess == pdFALSE )
    {
        pxCIO->print( xCommandDef_wifi.pcHelpString );
    }
}
