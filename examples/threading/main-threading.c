#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

/* Task handle is used to suspend the task */
static TaskHandle_t second_task_handle = NULL;

void vTask1(void *pvParameters)
{
    printf("Task 1 started\n");
    for (int i=0;i<4;i++)
    {
        printf("Task 1 is running\n");
        vTaskDelay(pdMS_TO_TICKS(2000));

        printf("Suspend second task\n");
        vTaskSuspend(second_task_handle);
        vTaskDelay(pdMS_TO_TICKS(2000));

        printf("Resume second task\n");
        vTaskResume(second_task_handle);
    }
    printf("Task 1 stopped\n");
    vTaskDelete(NULL);
}

void vTask2(void *pvParameters)
{
    printf("Task 2 started\n");
    for (;;)
    {
        printf("Task 2 is running\n");
        vTaskDelay(pdMS_TO_TICKS(500));
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

    xTaskCreate(vTask2,
                "Task 2",
                40,
                NULL,
                1,
                &second_task_handle);

    vTaskStartScheduler();
    return 0;
}