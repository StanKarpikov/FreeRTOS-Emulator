#pragma once

#include "esp_attr.h"
#include "soc/spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define portBYTE_ALIGNMENT			16

/* Type definitions. */
#define portCHAR		uint8_t
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		int32_t
#define portSHORT		int16_t
#define portSTACK_TYPE	uint8_t
#define portBASE_TYPE	int
// interrupt module will mask interrupt with priority less than threshold
#define RVHAL_EXCM_LEVEL    4

typedef portSTACK_TYPE			StackType_t;
typedef portBASE_TYPE			BaseType_t;
typedef unsigned portBASE_TYPE	UBaseType_t;
typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

#define portVALID_TCB_MEM(...) true
#define portVALID_STACK_MEM(ptr) true

#define pvPortMallocTcbMem(size)        malloc(size)
#define pvPortMallocStackMem(size)      malloc(size)

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
#ifdef CONFIG_FREERTOS_CHECK_PORT_CRITICAL_COMPLIANCE
#define portTRY_ENTER_CRITICAL(mux, timeout)
#define portENTER_CRITICAL(mux)
#define portEXIT_CRITICAL(mux)
#else
#define portTRY_ENTER_CRITICAL(mux, timeout)
#define portENTER_CRITICAL(mux)
#define portEXIT_CRITICAL(mux)
#endif /* CONFIG_FREERTOS_CHECK_PORT_CRITICAL_COMPLIANCE */

#define portTRY_ENTER_CRITICAL_ISR(mux, timeout)
#define portENTER_CRITICAL_ISR(mux)
#define portEXIT_CRITICAL_ISR(mux)

#define portTRY_ENTER_CRITICAL_SAFE(mux, timeout)
#define portENTER_CRITICAL_SAFE(mux)
#define portEXIT_CRITICAL_SAFE(mux)

// ---------------------- Yielding -------------------------

#define portYIELD()

/**
 * @note    The macro below could be used when passing a single argument, or without any argument,
 *          it was developed to support both usages of portYIELD inside of an ISR. Any other usage form
 *          might result in undesired behavior
 *
 * @note [refactor-todo] Refactor this to avoid va_args
 */
#if defined(__cplusplus) && (__cplusplus >  201703L)
#define portYIELD_FROM_ISR(...)
#else
#define portYIELD_FROM_ISR(...)
#endif

/* Yielding within an API call (when interrupts are off), means the yield should be delayed
   until interrupts are re-enabled.

   To do this, we use the "cross-core" interrupt as a trigger to yield on this core when interrupts are re-enabled.This
   is the same interrupt & code path which is used to trigger a yield between CPUs, although in this case the yield is
   happening on the same CPU.
*/
#define portYIELD_WITHIN_API()

#define xPortInIsrContext() false

#define portDISABLE_INTERRUPTS(...)

//#define taskCHECK_FOR_STACK_OVERFLOW()
//#define taskFIRST_CHECK_FOR_STACK_OVERFLOW()
//#define taskSECOND_CHECK_FOR_STACK_OVERFLOW()

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )  void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )        void vFunction( void *pvParameters )

#define portTICK_PERIOD_MS (10)

static inline BaseType_t IRAM_ATTR xPortGetCoreID(void)
{
    return (uint32_t) 0;
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

#ifdef __cplusplus
}
#endif
