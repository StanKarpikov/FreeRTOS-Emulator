/*
    FreeRTOS V10 - Copyright (C) 2021 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

	***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
	***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
	the FAQ page "My application does not run, what could be wrong?".  Have you
	defined configASSERT()?

	http://www.FreeRTOS.org/support - In return for receiving this top quality
	embedded software for free we request you assist our global community by
	participating in the support forum.

	http://www.FreeRTOS.org/training - Investing in training allows your team to
	be as productive as possible as early as possible.  Now you can receive
	FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
	Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define portNUM_PROCESSORS                              1

#define portUSING_MPU_WRAPPERS                          0
#define configUSE_MUTEX                                 1

#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1
#define configTHREAD_LOCAL_STORAGE_DELETE_CALLBACKS     1

/* configASSERT behaviour */
#ifndef __ASSEMBLER__
#include <assert.h>

#define UNTESTED_FUNCTION()

#endif /* def __ASSEMBLER__ */

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * Note that the default heap size is deliberately kept small so that
 * the build is more likely to succeed for configurations with limited
 * memory.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                            1
#define configUSE_IDLE_HOOK                             1
#define configUSE_TICK_HOOK                             1
#define configRECORD_STACK_HIGH_ADDRESS                 1
#define configTICK_RATE_HZ                              ( 1000 )

/* This has impact on speed of search for highest priority */
#define configMAX_PRIORITIES                            ( 25 )

/* Various things that impact minimum stack sizes */

/* Higher stack checker modes cause overhead on each function call */
#define configSTACK_OVERHEAD_CHECKER                    0

/* with optimizations disabled, scheduler uses additional stack */
#define configSTACK_OVERHEAD_OPTIMIZATION               0

/* apptrace mdule increases minimum stack usage */
#define configSTACK_OVERHEAD_APPTRACE                   0

/* Stack watchpoint decreases minimum usable stack size by up to 60 bytes.
   See FreeRTOS FREERTOS_WATCHPOINT_END_OF_STACK option in Kconfig. */
#define configSTACK_OVERHEAD_WATCHPOINT                   0

#define configSTACK_OVERHEAD_TOTAL (                                    \
                                    configSTACK_OVERHEAD_CHECKER +      \
                                    configSTACK_OVERHEAD_OPTIMIZATION + \
                                    configSTACK_OVERHEAD_APPTRACE +     \
                                    configSTACK_OVERHEAD_WATCHPOINT     \
                                                                        )

#define configMINIMAL_STACK_SIZE                        (768 + configSTACK_OVERHEAD_TOTAL)

#ifndef configIDLE_TASK_STACK_SIZE
#define configIDLE_TASK_STACK_SIZE 768
#endif

/* Minimal heap size to make sure examples can run on memory limited
   configs. Adjust this to suit your system. */


//We define the heap to span all of the non-statically-allocated shared RAM. ToDo: Make sure there
//is some space left for the app and main cpu when running outside of a thread.
#define configAPPLICATION_ALLOCATED_HEAP                1
#define configTOTAL_HEAP_SIZE                           (&_heap_end - &_heap_start)//( ( size_t ) (64 * 1024) )

#define configMAX_TASK_NAME_LEN                         ( 16 )

#define configBENCHMARK                                 0
#define configUSE_16_BIT_TICKS                          0
#define configIDLE_SHOULD_YIELD                         0
#define configQUEUE_REGISTRY_SIZE                       0

#define configUSE_MUTEXES                               1
#define configUSE_RECURSIVE_MUTEXES                     1
#define configUSE_COUNTING_SEMAPHORES                   1

#define configCHECK_FOR_STACK_OVERFLOW                  2

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                           0
#define configMAX_CO_ROUTINE_PRIORITIES                 ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
   to exclude the API function. */

#define INCLUDE_vTaskPrioritySet                        1
#define INCLUDE_uxTaskPriorityGet                       1
#define INCLUDE_vTaskDelete                             1
#define INCLUDE_vTaskCleanUpResources                   0
#define INCLUDE_vTaskSuspend                            1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_vTaskDelay                              1
#define INCLUDE_uxTaskGetStackHighWaterMark             1
#define INCLUDE_pcTaskGetTaskName                       1
#define INCLUDE_xTaskGetIdleTaskHandle                  1
#define INCLUDE_pxTaskGetStackStart                     1
#define INCLUDE_eTaskGetState                           1
#define INCLUDE_xTaskAbortDelay                         1
#define INCLUDE_xTaskGetHandle                          1
#define INCLUDE_xSemaphoreGetMutexHolder                1
#define INCLUDE_xTimerPendFunctionCall                  1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle          0   //Currently there is no need for this API

/* The priority at which the tick interrupt runs.  This should probably be
   kept at 1. */
#define configKERNEL_INTERRUPT_PRIORITY                 1

#define configUSE_NEWLIB_REENTRANT                      0

#define configSUPPORT_DYNAMIC_ALLOCATION                1
#define configSUPPORT_STATIC_ALLOCATION                 1

/* Test FreeRTOS timers (with timer task) and more. */
/* Some files don't compile if this flag is disabled */
#define configUSE_TIMERS                                1
#define configTIMER_TASK_PRIORITY                       1
#define configTIMER_QUEUE_LENGTH                        10
#define configTIMER_TASK_STACK_DEPTH                    2048

#define configUSE_QUEUE_SETS                            1

#define configUSE_TICKLESS_IDLE                         0
#if configUSE_TICKLESS_IDLE
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP           
#endif //configUSE_TICKLESS_IDLE


#define configENABLE_TASK_SNAPSHOT                      1

#ifndef configENABLE_TASK_SNAPSHOT
#define configENABLE_TASK_SNAPSHOT                      0
#endif

#define configCHECK_MUTEX_GIVEN_BY_OWNER                1

#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H       1

#define configTASK_NOTIFICATION_ARRAY_ENTRIES           1

// backward compatibility for 4.4
#define xTaskRemoveFromUnorderedEventList vTaskRemoveFromUnorderedEventList

#define configNUM_CORES                                 portNUM_PROCESSORS

#define	SLIST_FOREACH_SAFE(var, head, field, tvar)			\
    for ((var) = SLIST_FIRST((head));				\
        (var) && ((tvar) = SLIST_NEXT((var), field), 1);		\
        (var) = (tvar))
typedef void (*TaskFunction_t)( void * );


#endif /* FREERTOS_CONFIG_H */
