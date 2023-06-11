/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

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

#ifndef PORTMACRO_H
#define PORTMACRO_H

/******************************************************************************
	Max and Min Tick Rates (taken from all available ports):

  Max tick rate (Hz):  1000  Mili-Secs:  1.0
  Min tick rate (Hz):  100  Mili-Secs:  10.0

******************************************************************************/



/******************************************************************************
	Defines
******************************************************************************/
/* Type definitions. */
#include <stddef.h>
#include <stdint.h>
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	size_t
#define portBASE_TYPE	long
#define portPOINTER_SIZE_TYPE size_t

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;


#if( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL

	/* 32/64-bit tick type on a 32/64-bit architecture, so reads of the tick
	count do not need to be guarded with a critical section. */
	#define portTICK_TYPE_IS_ATOMIC 1
#endif

/* Hardware specifics. */
#define portSTACK_GROWTH			( -1 )
#define portTICK_PERIOD_MS			( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portINLINE __inline

#if defined( __x86_64__) || defined( _M_X64 )
	#define portBYTE_ALIGNMENT		8
#else
	#define portBYTE_ALIGNMENT		4
#endif

void vPortYield();
#define portYIELD()					vPortYield()

/* Simulated interrupts return pdFALSE if no context switch should be performed,
or a non-zero number if a context switch should be performed. */
#define portYIELD_FROM_ISR( x ) return x

void vPortCloseRunningThread( void *pvTaskToDelete, volatile BaseType_t *pxPendYield );
void vPortDeleteThread( void *pvThreadToDelete );
#define portCLEAN_UP_TCB( pxTCB )	vPortDeleteThread( pxTCB )
#define portPRE_TASK_DELETE_HOOK( pvTaskToDelete, pxPendYield ) vPortCloseRunningThread( ( pvTaskToDelete ), ( pxPendYield ) )
#define portDISABLE_INTERRUPTS() vPortEnterCritical()
#define portENABLE_INTERRUPTS() vPortExitCritical()

#define xPortInIsrContext() false

/* Critical section handling. */

#define portENTER_CRITICAL(x)		vPortEnterCritical(&global_mux, SPINLOCK_WAIT_FOREVER)
#define portEXIT_CRITICAL(x)			vPortExitCritical(&global_mux)

#define portTRY_ENTER_CRITICAL_ISR(mux, timeout) vPortEnterCritical(mux, timeout)
#define portENTER_CRITICAL_ISR(mux) vPortEnterCritical(mux, SPINLOCK_WAIT_FOREVER)
#define portEXIT_CRITICAL_ISR(mux) vPortExitCritical(mux)

#define portTRY_ENTER_CRITICAL_SAFE(mux, timeout) vPortEnterCritical(mux, timeout)
#define portENTER_CRITICAL_SAFE(mux) vPortEnterCritical(mux, SPINLOCK_WAIT_FOREVER)
#define portEXIT_CRITICAL_SAFE(mux) vPortExitCritical(mux)

#define portVALID_TCB_MEM(...) true
#define portVALID_STACK_MEM(ptr) true

#define pvPortMallocTcbMem(size)        malloc(size)
#define pvPortMallocStackMem(size)      malloc(sizeof(StackType_t)*size)

static inline BaseType_t xPortGetCoreID(void)
{
    return 0;
}
/*
 * Send an interrupt to another core in order to make the task running
 * on it yield for a higher-priority task.
 */
static inline void vPortYieldOtherCore(BaseType_t coreid)
{
    (void)coreid;
}

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

	/* Check the configuration. */
	#if( configMAX_PRIORITIES > 32 )
		#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
	#endif

	/* Store/clear the ready priorities in a bit map. */
	#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
	#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )


	/*-----------------------------------------------------------*/

	#ifdef __GNUC__
		#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )	\
			__asm volatile(	"bsr %1, %0\n\t" 									\
							:"=r"(uxTopPriority) : "rm"(uxReadyPriorities) : "cc" )
	#else
		/* BitScanReverse returns the bit position of the most significant '1'
		in the word. */
		#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) _BitScanReverse( ( DWORD * ) &( uxTopPriority ), ( uxReadyPriorities ) )
	#endif /* __GNUC__ */

#endif /* taskRECORD_READY_PRIORITY */

#ifndef __GNUC__
	__pragma( warning( disable:4211 ) ) /* Nonstandard extension used, as extern is only nonstandard to MSVC. */
#endif


/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void * pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void * pvParameters )

#define portINTERRUPT_YIELD				( 0UL )
#define portINTERRUPT_TICK				( 1UL )

/*
 * Raise a simulated interrupt represented by the bit mask in ulInterruptMask.
 * Each bit can be used to represent an individual interrupt - with the first
 * two bits being used for the Yield and Tick interrupts respectively.
*/
void vPortGenerateSimulatedInterrupt( uint32_t ulInterruptNumber );

/*
 * Install an interrupt handler to be called by the simulated interrupt handler
 * thread.  The interrupt number must be above any used by the kernel itself
 * (at the time of writing the kernel was using interrupt numbers 0, 1, and 2
 * as defined above).  The number must also be lower than 32.
 *
 * Interrupt handler functions must return a non-zero value if executing the
 * handler resulted in a task switch being required.
 */
void vPortSetInterruptHandler( uint32_t ulInterruptNumber, uint32_t (*pvHandler)( void ) );


#define THREAD_TASK_IDLE_PRIO    QThread::IdlePriority
#define THREAD_TASK_RUNNING_PRIO QThread::HighPriority
#define THREAD_SV_TIMER_PRIO     QThread::HighestPriority
#define THREAD_IRQ_PRIO          QThread::TimeCriticalPriority

/* ESP32 Modifications */
#if ESP_PLATFORM
#include "esp_attr.h"
#include "soc/spinlock.h"

typedef spinlock_t                          portMUX_TYPE;               /**< Spinlock type used by FreeRTOS critical sections */
#define portMUX_INITIALIZER_UNLOCKED        SPINLOCK_INITIALIZER        /**< Spinlock initializer */
#define portMUX_FREE_VAL                    SPINLOCK_FREE               /**< Spinlock is free. [refactor-todo] check if this is still required */
#define portMUX_NO_TIMEOUT                  SPINLOCK_WAIT_FOREVER       /**< When passed for 'timeout_cycles', spin forever if necessary. [refactor-todo] check if this is still required */
#define portMUX_TRY_LOCK                    SPINLOCK_NO_WAIT            /**< Try to acquire the spinlock a single time only. [refactor-todo] check if this is still required */
#define portMUX_INITIALIZE(mux)             spinlock_initialize(mux)    /*< Initialize a spinlock to its unlocked state */

extern portMUX_TYPE global_mux;

static inline void vPortEnterCritical( portMUX_TYPE *mux, int timeout )
{

}

static inline void vPortExitCritical( portMUX_TYPE *mux )
{

}

#include "esp_timer.h"

static inline unsigned long port_get_time_ms(void)
{
    return esp_timer_get_time()/1000;
}

#else

#include <time.h>

static inline unsigned long port_get_time_ms(void)
{
    return time(NULL);
}

#endif

/* Converts a time in milliseconds to a time in ticks.  This macro can be
 * overridden by a macro of the same name defined in FreeRTOSConfig.h in case the
 * definition here is not suitable for your application. */
#ifndef pdMS_TO_TICKS
    #define pdMS_TO_TICKS( xTimeInMs )    ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000U ) )
#endif
#ifndef pdTICKS_TO_MS
    #define pdTICKS_TO_MS( xTicks )       ( ( TickType_t ) ( ( uint64_t ) ( xTicks ) * 1000 / configTICK_RATE_HZ ) )
#endif

#endif

