#include "esp_err.h"
#include "esp_freertos_hooks.h"
#include "stdlib.h"
#include "portable.h"
#include "FreeRTOS.h"        /* This pulls in portmacro.h */
#include "task.h"            /* Required for TaskHandle_t, tskNO_AFFINITY, and vTaskStartScheduler */

#if portUSING_MPU_WRAPPERS
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters, BaseType_t xRunPrivileged )
#else
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
#endif
{
    return pxTopOfStack;
}


BaseType_t xPortStartScheduler( void )
{
    portDISABLE_INTERRUPTS();
    // Interrupts are disabled at this point and stack contains PS with enabled interrupts when task context is restored

#if XCHAL_CP_NUM > 0
    /* Initialize co-processor management for tasks. Leave CPENABLE alone. */
    _xt_coproc_init();
#endif

    /* Setup the hardware to generate the tick. */
//    vPortSetupTimer();

    // Cannot be directly called from C; never returns
//    __asm__ volatile ("call0    _frxt_dispatch\n");

    /* Should not get here. */
    return true;
}

void vPortEndScheduler( void )
{
    /* It is unlikely that the Xtensa port will get stopped.  If required simply
    disable the tick interrupt here. */
    abort();
}

// ------------------- Hook Functions ----------------------

void  __attribute__((weak)) vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{

}

void vApplicationTickHook( void )
{

}

void vApplicationIdleHook( void )
{

}

esp_err_t esp_register_freertos_idle_hook_for_cpu(esp_freertos_idle_cb_t new_idle_cb, UBaseType_t cpuid)
{
    return ESP_OK;
}

void esp_deregister_freertos_idle_hook_for_cpu(esp_freertos_idle_cb_t old_idle_cb, UBaseType_t cpuid)
{

}

void heap_caps_enable_nonos_stack_heaps(void)
{

}
// ----------------------- System --------------------------
