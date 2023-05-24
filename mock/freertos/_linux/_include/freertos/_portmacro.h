#pragma once


/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright 2020 Cambridge Consultants Ltd.
 *
 * SPDX-License-Identifier: MIT
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


#ifndef PORTMACRO_H
#define PORTMACRO_H

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

#include <limits.h>

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  unsigned long
#define portBASE_TYPE   long
#define portPOINTER_SIZE_TYPE intptr_t

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

typedef unsigned long TickType_t;
#define portMAX_DELAY ( TickType_t ) ULONG_MAX

#define portTICK_TYPE_IS_ATOMIC 1

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portNORETURN               __attribute__( ( noreturn ) )

#define portSTACK_GROWTH            ( -1 )
#define portHAS_STACK_OVERFLOW_CHECKING ( 1 )
#define portTICK_PERIOD_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portTICK_RATE_MICROSECONDS  ( ( TickType_t ) 1000000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          8
#define portTICK_PERIOD_uS          portTICK_RATE_MICROSECONDS
/*-----------------------------------------------------------*/

/* Scheduler utilities. */
extern void vPortYield( void );

#define portYIELD() vPortYield()

//#define portEND_SWITCHING_ISR( xSwitchRequired ) if( xSwitchRequired != pdFALSE ) vPortYield()
//#define portYIELD_FROM_ISR( x ) portEND_SWITCHING_ISR( x )
#define portYIELD_FROM_ISR() vPortYield()
/*-----------------------------------------------------------*/

/* Critical section management. */
extern void vPortDisableInterrupts( void );
extern void vPortEnableInterrupts( void );
#define portSET_INTERRUPT_MASK()        ( vPortDisableInterrupts() )
#define portCLEAR_INTERRUPT_MASK()      ( vPortEnableInterrupts() )

extern portBASE_TYPE xPortSetInterruptMask( void );
extern void vPortClearInterruptMask( portBASE_TYPE xMask );

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
#define portSET_INTERRUPT_MASK_FROM_ISR()       xPortSetInterruptMask()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)    vPortClearInterruptMask(x)
#define portDISABLE_INTERRUPTS()                portSET_INTERRUPT_MASK()
#define portENABLE_INTERRUPTS()                 portCLEAR_INTERRUPT_MASK()
//#define portENTER_CRITICAL(x)                    vPortEnterCritical()
//#define portEXIT_CRITICAL(x)                     vPortExitCritical()

/*-----------------------------------------------------------*/

extern void vPortThreadDying( void *pxTaskToDelete, volatile BaseType_t *pxPendYield );
extern void vPortCancelThread( void *pxTaskToDelete );
#define portPRE_TASK_DELETE_HOOK( pvTaskToDelete, pxPendYield ) /*vPortThreadDying( ( pvTaskToDelete ), ( pxPendYield ) )*/
#define portCLEAN_UP_TCB( pxTCB )   /*vPortCancelThread( pxTCB )*/
/*-----------------------------------------------------------*/

#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/*
 * Tasks run in their own pthreads and context switches between them
 * are always a full memory barrier. ISRs are emulated as signals
 * which also imply a full memory barrier.
 *
 * Thus, only a compilier barrier is needed to prevent the compiler
 * reordering.
 */
#define portMEMORY_BARRIER() __asm volatile( "" ::: "memory" )

extern unsigned long ulPortGetRunTime( void );
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() /* no-op */
#define portGET_RUN_TIME_COUNTER_VALUE()         ulPortGetRunTime()

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif /* PORTMACRO_H */



#include "esp_attr.h"
#include "soc/spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define portFLOAT		float
#define portDOUBLE		double

// interrupt module will mask interrupt with priority less than threshold
#define RVHAL_EXCM_LEVEL    4

#define portVALID_TCB_MEM(...) true
#define portVALID_STACK_MEM(ptr) true

#define pvPortMallocTcbMem(size)        malloc(size)
#define pvPortMallocStackMem(size)      malloc(sizeof(StackType_t)*size)

#define portSTACK_GROWTH                ( -1 )

// ------------------ Critical Sections --------------------

/**
 * @brief FreeRTOS critical section macros
 *
 * - Added a spinlock argument for SMP
 * - Can be nested
 * - Compliance versions will assert if regular critical section API is used in ISR context
 * - Safe versions can be called from either contexts
 */

//#include <pthread.h>
//static pthread_mutex_t isr_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef CONFIG_FREERTOS_CHECK_PORT_CRITICAL_COMPLIANCE
#define portTRY_ENTER_CRITICAL(mux, timeout)
#define portENTER_CRITICAL(mux)
#define portEXIT_CRITICAL(mux)
#else
//#define portTRY_ENTER_CRITICAL(mux, timeout) pthread_mutex_lock(&isr_mutex)
//#define portEXIT_CRITICAL(mux) pthread_mutex_unlock(&isr_mutex)
#define portTRY_ENTER_CRITICAL(mux, timeout) /*spinlock_acquire(mux, timeout, __LINE__, __FILE__)*/

//#define portENTER_CRITICAL(mux)  /*spinlock_acquire(mux, SPINLOCK_WAIT_FOREVER, __LINE__, __FILE__)*/
//#define portEXIT_CRITICAL(mux) /*spinlock_release(mux)*/

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
#define portENTER_CRITICAL(mux)        vPortEnterCritical()
#define portEXIT_CRITICAL(mux)         vPortExitCritical()

#endif /* CONFIG_FREERTOS_CHECK_PORT_CRITICAL_COMPLIANCE */

//#define portTRY_ENTER_CRITICAL_ISR(mux, timeout) pthread_mutex_lock(&isr_mutex)
//#define portENTER_CRITICAL_ISR(mux)  pthread_mutex_lock(&isr_mutex)
//#define portEXIT_CRITICAL_ISR(mux) pthread_mutex_unlock(&isr_mutex)

//#define portTRY_ENTER_CRITICAL_SAFE(mux, timeout) pthread_mutex_lock(&isr_mutex)
//#define portENTER_CRITICAL_SAFE(mux) pthread_mutex_lock(&isr_mutex)
//#define portEXIT_CRITICAL_SAFE(mux) pthread_mutex_unlock(&isr_mutex)

#define portTRY_ENTER_CRITICAL_ISR(mux, timeout) /* spinlock_acquire(mux, timeout, __LINE__, __FILE__)*/
#define portENTER_CRITICAL_ISR(mux)  /* spinlock_acquire(mux, SPINLOCK_WAIT_FOREVER, __LINE__, __FILE__)*/
#define portEXIT_CRITICAL_ISR(mux) /* spinlock_release(mux)*/

#define portTRY_ENTER_CRITICAL_SAFE(mux, timeout) /* spinlock_acquire(mux, timeout, __LINE__, __FILE__)*/
#define portENTER_CRITICAL_SAFE(mux) /* spinlock_acquire(mux, SPINLOCK_WAIT_FOREVER, __LINE__, __FILE__)*/
#define portEXIT_CRITICAL_SAFE(mux) /* spinlock_release(mux) */

// ---------------------- Yielding -------------------------

/* Yielding within an API call (when interrupts are off), means the yield should be delayed
   until interrupts are re-enabled.

   To do this, we use the "cross-core" interrupt as a trigger to yield on this core when interrupts are re-enabled.This
   is the same interrupt & code path which is used to trigger a yield between CPUs, although in this case the yield is
   happening on the same CPU.
*/
#define portYIELD_WITHIN_API()

#define xPortInIsrContext() false

//#define taskCHECK_FOR_STACK_OVERFLOW()
//#define taskFIRST_CHECK_FOR_STACK_OVERFLOW()
//#define taskSECOND_CHECK_FOR_STACK_OVERFLOW()

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )  void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )        void vFunction( void *pvParameters )

static inline BaseType_t IRAM_ATTR xPortGetCoreID(void)
{
    return 0;
}
/*
 * Send an interrupt to another core in order to make the task running
 * on it yield for a higher-priority task.
 */
static inline void vPortYieldOtherCore(BaseType_t coreid)
{

}

/* ---------------------- Spinlocks ------------------------
 * - Modifications made to critical sections to support SMP
 * - See "Critical Sections & Disabling Interrupts" in docs/api-guides/freertos-smp.rst for more details
 * - Remark: For the ESP32, portENTER_CRITICAL and portENTER_CRITICAL_ISR both alias vPortEnterCritical, meaning that
 *           either function can be called both from ISR as well as task context. This is not standard FreeRTOS
 *           behaviorr; please keep this in mind if you need any compatibility with other FreeRTOS implementations.
 * @note [refactor-todo] Check if these comments are still true
 * ------------------------------------------------------ */

typedef spinlock_t                          portMUX_TYPE;               /**< Spinlock type used by FreeRTOS critical sections */
#define portMUX_INITIALIZER_UNLOCKED        SPINLOCK_INITIALIZER        /**< Spinlock initializer */
#define portMUX_FREE_VAL                    SPINLOCK_FREE               /**< Spinlock is free. [refactor-todo] check if this is still required */
#define portMUX_NO_TIMEOUT                  SPINLOCK_WAIT_FOREVER       /**< When passed for 'timeout_cycles', spin forever if necessary. [refactor-todo] check if this is still required */
#define portMUX_TRY_LOCK                    SPINLOCK_NO_WAIT            /**< Try to acquire the spinlock a single time only. [refactor-todo] check if this is still required */
#define portMUX_INITIALIZE(mux)             spinlock_initialize(mux)    /*< Initialize a spinlock to its unlocked state */

typedef void (*TaskFunction_t)( void * );

//extern void vPortForciblyEndThread( void *pxTaskToDelete );
//#define traceTASK_DELETE( pxTaskToDelete )      vPortForciblyEndThread( pxTaskToDelete )

//extern void vPortAddTaskHandle( void *pxTaskHandle );
//#define traceTASK_CREATE( pxNewTCB )            vPortAddTaskHandle( pxNewTCB )

/* Posix Signal definitions that can be changed or read as appropriate. */
#define SIG_SUSPEND                 SIGUSR1
#define SIG_RESUME                  SIGUSR2

/* Enable the following hash defines to make use of the real-time tick where time progresses at real-time. */
#define SIG_TICK                    SIGALRM
#define TIMER_TYPE                  ITIMER_REAL
/* Enable the following hash defines to make use of the process tick where time progresses only when the process is executing.
#define SIG_TICK                    SIGVTALRM
#define TIMER_TYPE                  ITIMER_VIRTUAL      */
/* Enable the following hash defines to make use of the profile tick where time progresses when the process or system calls are executing.
#define SIG_TICK                    SIGPROF
#define TIMER_TYPE                  ITIMER_PROF */

//void vPortSystemTickHandler(int sig);

#ifdef __cplusplus
}
#endif
