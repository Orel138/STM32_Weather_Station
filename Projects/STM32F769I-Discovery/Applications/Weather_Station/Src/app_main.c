/*
 * FreeRTOS STM32 Reference Integration
 *
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

 /*
 * Modifications Copyright (c) 2024 Orel138
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

#include "logging_levels.h"
#define LOG_LEVEL LOG_DEBUG
#include "logging.h"
#include "main.h"
#include "sys_evt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "kvstore.h"

//#include "appli_flash_layout.h"

//#include "flash.h"
#include "cli.h"
#include "lfs.h"
#include "lfs_port.h"

//#include "stdlib.h" /* rand() */

//#include "low_level_ext_flash.h"

#include "dht11.h"

#include "i_nucleo_lrwan1_pressure.h"

static lfs_t * pxLfsCtx = NULL;

EventGroupHandle_t xSystemEvents = NULL;

lfs_t * pxGetDefaultFsCtx( void )
{
    while( pxLfsCtx == NULL )
    {
        LogDebug( "Waiting for FS Initialization." );
        /* Wait for FS to be initialized */
        vTaskDelay( 1000 );
    }
    return pxLfsCtx;
}

//static int fs_init( void )
//{
//    static lfs_t xLfsCtx = { 0 };
//
//    struct lfs_info xDirInfo = { 0 };
//
//    /* Block time of up to 1 s for filesystem to initialize */
//    //const struct lfs_config * pxCfg = pxInitializeOSPIFlashFs( pdMS_TO_TICKS( 30 * 1000 ) );
//
//    /* mount the filesystem */
//    int err = lfs_mount( &xLfsCtx, pxCfg );
//
//    /* format if we can't mount the filesystem
//     * this should only happen on the first boot
//     */
//    if( err != LFS_ERR_OK )
//    {
//        LogError( "Failed to mount partition. Formatting..." );
//        err = lfs_format( &xLfsCtx, pxCfg );
//
//        if( err == 0 )
//        {
//            err = lfs_mount( &xLfsCtx, pxCfg );
//        }
//
//        if( err != LFS_ERR_OK )
//        {
//            LogError( "Failed to format littlefs device." );
//        }
//    }
//
//    if( lfs_stat( &xLfsCtx, "/cfg", &xDirInfo ) == LFS_ERR_NOENT )
//    {
//        err = lfs_mkdir( &xLfsCtx, "/cfg" );
//
//        if( err != LFS_ERR_OK )
//        {
//            LogError( "Failed to create /cfg directory." );
//        }
//    }
//
//    if( lfs_stat( &xLfsCtx, "/ota", &xDirInfo ) == LFS_ERR_NOENT )
//    {
//        err = lfs_mkdir( &xLfsCtx, "/ota" );
//
//        if( err != LFS_ERR_OK )
//        {
//            LogError( "Failed to create /ota directory." );
//        }
//    }
//
//    if( err == 0 )
//    {
//        /* Export the FS context */
//        pxLfsCtx = &xLfsCtx;
//    }
//
//    return err;
//}

static void vHeartbeatTask( void * pvParameters )
{
    ( void ) pvParameters;
    
    HAL_GPIO_WritePin( LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET );

    while(1)
    {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );

        HAL_GPIO_TogglePin( LED1_GPIO_Port, LED1_Pin );
    }
}

//extern void vImageValidationTask( void * pvParameters );
//extern void net_main( void * pvParameters );
//extern void vMQTTAgentTask( void * );
//extern void vSensorPublishTask( void * );
//extern void vShadowDeviceTask( void * );
//extern void vOTAUpdateTask( void * pvParam );
//extern void vDefenderAgentTask( void * );
//extern void vSensorDataPublishTask( void * pvParameters );
//extern void vShadowUpdateTask( void * pvParameters );

void vInitTask( void * pvArgs )
{
    BaseType_t xResult;
    int xMountStatus;

    ( void ) pvArgs;
    
    xResult = xTaskCreate( Task_CLI, "cli", 2048, NULL, 10, NULL );
    configASSERT( xResult == pdTRUE );

//    xMountStatus = fs_init();
//
//	if( xMountStatus == LFS_ERR_OK )
//	{
//		/*
//		 * FIXME: Need to debug  the cause of internal flash status register error here.
//		 * Clearing the flash status register as a workaround.
//		 */
//		FLASH_WaitForLastOperation( 50000U ); // = FLASH_TIMEOUT_VALUE
//
//		LogInfo( "File System mounted." );
//
//		( void ) xEventGroupSetBits( xSystemEvents, EVT_MASK_FS_READY );
//
//		KVStore_init();
//	}
//	else
//	{
//		LogError( "Failed to mount filesystem." );
//	}

    xResult = xTaskCreate( vHeartbeatTask, "Heartbeat", 128, NULL, tskIDLE_PRIORITY, NULL );
    configASSERT( xResult == pdTRUE );

//   xResult = xTaskCreate( &vTaskDHT11, "DHT11", 1024, NULL, 23, NULL );
//   configASSERT( xResult == pdTRUE );
    
//    xResult = xTaskCreate( &net_main, "EthNet", 512, NULL, 23, NULL );
//    configASSERT( xResult == pdTRUE );
//
//    xResult = xTaskCreate( vMQTTAgentTask, "MQTTAgent", 1024, NULL, 10, NULL );
//    configASSERT( xResult == pdTRUE );
//
//    xResult = xTaskCreate( vOTAUpdateTask, "OTAUpdate", 4096, NULL, tskIDLE_PRIORITY + 1, NULL );
//    configASSERT( xResult == pdTRUE );
//
//    xResult = xTaskCreate( vSensorPublishTask, "Sense", 512, NULL, 6, NULL );
//    configASSERT( xResult == pdTRUE );
//
//    xResult = xTaskCreate( vShadowDeviceTask, "ShadowDevice", 512, NULL, 5, NULL );
//    configASSERT( xResult == pdTRUE );
//
//    xResult = xTaskCreate( vDefenderAgentTask, "AWSDefender", 512 , NULL, 5, NULL );
//    configASSERT( xResult == pdTRUE );

   float *pressure = NULL;
   BSP_PRESSURE_Get_Press(LPS22HB_P_0_handle, pressure);

//   LogSys("pressure : %.2f", pressure);
    
    while( 1 )
    {
        vTaskSuspend( NULL );
    }
}

static uint32_t ulCsrFlags = 0;

static void vDetermineResetSource()
{
  const char * pcResetSource = NULL;

  ulCsrFlags &= 0xFFFF0000;

  if ( ulCsrFlags != 0 )
  {
    if( ulCsrFlags == (RCC_CSR_BORRSTF | RCC_CSR_PINRSTF | RCC_CSR_PORRSTF ) )
    {
        pcResetSource = "Power-on reset";
    }
    else if( ulCsrFlags == RCC_CSR_PINRSTF )
    {
        pcResetSource = "Pin reset";
    }
    else if( ulCsrFlags == ( RCC_CSR_BORRSTF | RCC_CSR_PINRSTF ) )
    {
        pcResetSource = "Brownout reset (BOR reset)";
    }
    else if( ulCsrFlags == ( RCC_CSR_SFTRSTF | RCC_CSR_PINRSTF ) )
    {
        pcResetSource = "System reset generated by CPU (Software reset)";
    }
    else if( ulCsrFlags == ( RCC_CSR_WWDGRSTF | RCC_CSR_PINRSTF ) )
    {
        pcResetSource = "WWDG reset (Window watchdog)";
    }
    else if( ulCsrFlags == ( RCC_CSR_IWDGRSTF | RCC_CSR_PINRSTF ) )
    {
        pcResetSource = "IWDG reset (Independent watchdog)";
    }
    else if( ulCsrFlags == ( RCC_CSR_LPWRRSTF | RCC_CSR_PINRSTF ) )
    {
        pcResetSource = "CPU erroneously enters Stop or Standby mode (Low-power)";
    }
    else
    {
        pcResetSource = "Unknown";
    }

    LogSys( "Reset Source: 0x%x : %s.", ulCsrFlags, pcResetSource );
  }
}

uint32_t ulGetResetSource( void )
{
    return ulCsrFlags;
}


int main( void )
{
    ulCsrFlags = RCC->CSR;
    uint32_t valClockFreq = 0;

    hw_init();

    vLoggingInit();

    vDetermineResetSource();
    __HAL_RCC_CLEAR_RESET_FLAGS();

    valClockFreq = HAL_RCC_GetSysClockFreq();

    LogInfo( "HW Init Complete , Clock Freq = %d Hz", valClockFreq);
    
    xSystemEvents = xEventGroupCreate();

    xTaskCreate( vInitTask, "Init", 2048, NULL, 8, NULL );

    /* Start scheduler */
    vTaskStartScheduler();

    /* Initialize threads */

    LogError( "Kernel start returned." );

    /* This loop should be inaccessible.*/
    while( 1 )
    {
        __NOP();
    }
}

//UBaseType_t uxRand( void )
//{
//  return (UBaseType_t) rand();
//}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/*-----------------------------------------------------------*/

#if configUSE_IDLE_HOOK == 1
void vApplicationIdleHook( void )
{
    vPetWatchdog();
}
#endif /* configUSE_IDLE_HOOK == 1 */

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    LogError( "Malloc failed" );

    while( 1 )
    {
        __NOP();
    }
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName )

{
    volatile uint32_t ulSetToZeroToStepOut = 1UL;

    taskENTER_CRITICAL();

    LogSys( "Stack overflow in %s", pcTaskName ); /* WARN: The log message will not be output until ulSetToZeroToStepOut is reset by the user. */
    ( void ) xTask;

    while( ulSetToZeroToStepOut != 0 )
    {
        __NOP();
    }

    taskEXIT_CRITICAL();
}

void vDoSystemReset( void )
{
    vPetWatchdog();

    if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING )
    {
        vTaskSuspendAll();
    }

    LogSys( "System Reset in progress." );

    /* Drain log buffers */
    vDyingGasp();

    NVIC_SystemReset();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == USER_BUTTON_Pin) {
    HAL_GPIO_TogglePin( LED2_GPIO_Port, LED2_Pin);

    uint8_t *who_am_i;
    if ( BSP_PRESSURE_Get_WhoAmI(LPS22HB_P_0_handle, who_am_i) == COMPONENT_OK)
    {
    	__NOP();
    }

  } else {
      __NOP();
  }
}
