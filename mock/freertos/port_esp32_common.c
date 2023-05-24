#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void  __attribute__((weak)) vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{

}

void vApplicationTickHook( void )
{

}

void vApplicationIdleHook( void )
{
//    prctl(PR_SET_NAME, "Idle", 0, 0, 0);
}


void heap_caps_enable_nonos_stack_heaps(void)
{

}
