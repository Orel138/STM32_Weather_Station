/*
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Derived from simple_sub_pub_demo.c
 *
 * Portions:
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
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
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */


#include "logging_levels.h"
/* define LOG_LEVEL here if you want to modify the logging level from the default */

#define LOG_LEVEL    LOG_DEBUG

#include "logging.h"

/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "kvstore.h"

/* MQTT library includes. */
#include "core_mqtt.h"
#include "core_mqtt_agent.h"
#include "sys_evt.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

/* Telemetry inputs */
#include "lwip/stats.h"

#define MQTT_PUBLISH_MAX_LEN                 ( 512 )
#define MQTT_PUBLISH_TIME_BETWEEN_MS         ( 2000 )
#define MQTT_PUBLISH_TOPIC                   "net_stats"
#define MQTT_PUBLICH_TOPIC_STR_LEN           ( 256 )
#define MQTT_PUBLISH_BLOCK_TIME_MS           ( 3000 )
#define MQTT_PUBLISH_NOTIFICATION_WAIT_MS    ( 3000 )

#define MQTT_NOTIFY_IDX                      ( 1 )
#define MQTT_PUBLISH_QOS                     ( MQTTQoS0 )

/*-----------------------------------------------------------*/

/**
 * @brief Defines the structure to use as the command callback context in this
 * demo.
 */
struct MQTTAgentCommandContext
{
    MQTTStatus_t xReturnStatus;
    TaskHandle_t xTaskToNotify;
};

typedef struct
{
    int32_t timer;
    int32_t sent;
    int32_t received;
} NetworkData_t;

/*-----------------------------------------------------------*/

static void prvPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                       MQTTAgentReturnInfo_t * pxReturnInfo )
{
    configASSERT( pxCommandContext != NULL );
    configASSERT( pxReturnInfo != NULL );

    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        /* Send the context's ulNotificationValue as the notification value so
         * the receiving task can check the value it set in the context matches
         * the value it receives in the notification. */
        ( void ) xTaskNotifyGiveIndexed( pxCommandContext->xTaskToNotify,
                                         MQTT_NOTIFY_IDX );
    }
}

/*-----------------------------------------------------------*/

static BaseType_t prvPublishAndWaitForAck( MQTTAgentHandle_t xAgentHandle,
                                           const char * pcTopic,
                                           const void * pvPublishData,
                                           size_t xPublishDataLen )
{
    BaseType_t xResult = pdFALSE;
    MQTTStatus_t xStatus;

    configASSERT( pcTopic != NULL );
    configASSERT( pvPublishData != NULL );
    configASSERT( xPublishDataLen > 0 );

    MQTTPublishInfo_t xPublishInfo =
    {
        .qos             = MQTT_PUBLISH_QOS,
        .retain          = 0,
        .dup             = 0,
        .pTopicName      = pcTopic,
        .topicNameLength = strlen( pcTopic ),
        .pPayload        = pvPublishData,
        .payloadLength   = xPublishDataLen
    };

    MQTTAgentCommandContext_t xCommandContext =
    {
        .xTaskToNotify = xTaskGetCurrentTaskHandle(),
        .xReturnStatus = MQTTIllegalState,
    };

    MQTTAgentCommandInfo_t xCommandParams =
    {
        .blockTimeMs                 = MQTT_PUBLISH_BLOCK_TIME_MS,
        .cmdCompleteCallback         = prvPublishCommandCallback,
        .pCmdCompleteCallbackContext = &xCommandContext,
    };

    /* Clear the notification index */
    xTaskNotifyStateClearIndexed( NULL, MQTT_NOTIFY_IDX );


    xStatus = MQTTAgent_Publish( xAgentHandle,
                                 &xPublishInfo,
                                 &xCommandParams );

    if( xStatus == MQTTSuccess )
    {
        xResult = ulTaskNotifyTakeIndexed( MQTT_NOTIFY_IDX,
                                           pdTRUE,
                                           pdMS_TO_TICKS( MQTT_PUBLISH_NOTIFICATION_WAIT_MS ) );

        if( xResult == 0 )
        {
            LogError( "Timed out while waiting for publish ACK or Sent event. xTimeout = %d",
                      pdMS_TO_TICKS( MQTT_PUBLISH_NOTIFICATION_WAIT_MS ) );
            xResult = pdFALSE;
        }
        else if( xCommandContext.xReturnStatus != MQTTSuccess )
        {
            LogError( "MQTT Agent returned error code: %d during publish operation.",
                      xCommandContext.xReturnStatus );
            xResult = pdFALSE;
        }
    }
    else
    {
        LogError( "MQTTAgent_Publish returned error code: %d.",
                  xStatus );
    }

    return xResult;
}

static BaseType_t xIsMqttConnected( void )
{
    /* Wait for MQTT to be connected */
    EventBits_t uxEvents = xEventGroupWaitBits( xSystemEvents,
                                                EVT_MASK_MQTT_CONNECTED,
                                                pdFALSE,
                                                pdTRUE,
                                                0 );

    return( ( uxEvents & EVT_MASK_MQTT_CONNECTED ) == EVT_MASK_MQTT_CONNECTED );
}

/*-----------------------------------------------------------*/


static BaseType_t xUpdateData( NetworkData_t * pxData )
{
    pxData->timer = (uwTick/1000) % 30; /* Linear reload every 30 s. */
    pxData->sent = lwip_stats.ip.xmit;
    pxData->received = lwip_stats.ip.recv;
    return pdTRUE;
}

/*-----------------------------------------------------------*/

#if defined ( __ICCARM__ ) /* IAR Compiler */
size_t strlcat_iar_impl(char * dest, const char * src, size_t size);

size_t strlcat_iar_impl(char * dest, const char * src, size_t size) {
    size_t src_length = strlen (src);
    
    if (size == 0) return src_length;
    
    size_t dest_length = strnlen(dest, size);
    
    if (dest_length != size)
    {
    size_t to_copy = size - dest_length - 1;

    if (to_copy > src_length)
    to_copy = src_length;

    char *target = dest + dest_length;
    memcpy (target, src, to_copy);
    target[to_copy] = '\0';
    }

    configASSERT(sizeof (uintptr_t) == sizeof (size_t));

    return dest_length + src_length;
}
#endif

void vSensorPublishTask( void * pvParameters )
{
    BaseType_t xResult = pdFALSE;
    BaseType_t xExitFlag = pdFALSE;
    char payloadBuf[ MQTT_PUBLISH_MAX_LEN ];
    MQTTAgentHandle_t xAgentHandle = NULL;
    char pcTopicString[ MQTT_PUBLICH_TOPIC_STR_LEN ] = { 0 };
    size_t uxTopicLen = 0;

    ( void ) pvParameters;

    uxTopicLen = KVStore_getString( CS_CORE_THING_NAME, pcTopicString, MQTT_PUBLICH_TOPIC_STR_LEN );

    if( uxTopicLen > 0 )
    {     
#if defined ( __ICCARM__ ) /* IAR Compiler */
        uxTopicLen = strlcat_iar_impl( pcTopicString, "/" MQTT_PUBLISH_TOPIC, MQTT_PUBLICH_TOPIC_STR_LEN );
#else
        uxTopicLen = strlcat( pcTopicString, "/" MQTT_PUBLISH_TOPIC, MQTT_PUBLICH_TOPIC_STR_LEN );
#endif /* IAR Compiler */
    }

    if( ( uxTopicLen == 0 ) || ( uxTopicLen >= MQTT_PUBLICH_TOPIC_STR_LEN ) )
    {
        LogError( "Failed to construct topic string." );
        xExitFlag = pdTRUE;
    }

    vSleepUntilMQTTAgentReady();

    xAgentHandle = xGetMqttAgentHandle();

    while( xExitFlag == pdFALSE )
    {
        TickType_t xTicksToWait = pdMS_TO_TICKS( MQTT_PUBLISH_TIME_BETWEEN_MS );
        TimeOut_t xTimeOut;

        vTaskSetTimeOutState( &xTimeOut );

        NetworkData_t xData;
        xResult = xUpdateData( &xData );

        if( xResult != pdTRUE )
        {
            LogError( "Error while reading sensor data." );
        }
        else if( xIsMqttConnected() == pdTRUE )
        {
            int bytesWritten = 0;

            /* Write to */
            bytesWritten = snprintf( payloadBuf,
                                     MQTT_PUBLISH_MAX_LEN,
                                     "{ \"timer_s\": %ld, \"sent_p\": %ld, \"received_p\": %ld }",
                                     xData.timer, xData.sent, xData.received );

            if( bytesWritten < MQTT_PUBLISH_MAX_LEN )
            {
                xResult = prvPublishAndWaitForAck( xAgentHandle,
                                                   pcTopicString,
                                                   payloadBuf,
                                                   bytesWritten );
            }
            else if( bytesWritten > 0 )
            {
                LogError( "Not enough buffer space." );
            }
            else
            {
                LogError( "Printf call failed." );
            }

            if( xResult == pdTRUE )
            {
                LogDebug( payloadBuf );
            }
        }

        /* Adjust remaining tick count */
        if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE )
        {
            /* Wait until its time to poll the sensors again */
            vTaskDelay( xTicksToWait );
        }
    }
}
