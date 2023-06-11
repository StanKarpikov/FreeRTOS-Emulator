#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stdio.h"

static TimerHandle_t xTimer;

void vTimerCallback( TimerHandle_t xTimer )
{
    printf("Timer callback\n");
}

void vTask1(void *pvParameters)
{
    xTimer = xTimerCreate("Timer",
                          pdMS_TO_TICKS(1500),
                          pdTRUE,
                          ( void * ) 0,
                          vTimerCallback);

    xTimerStart( xTimer, 0 );

    while(1)
    {
        printf("Task 1 is running\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main(void)
{
    xTaskCreate(vTask1,  
                "Task 1",
                40,
                NULL,
                1,
                NULL);

    vTaskStartScheduler();
    return 0;
}