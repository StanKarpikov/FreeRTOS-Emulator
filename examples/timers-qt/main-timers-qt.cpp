#include <QDebug>
#include <QApplication>
#include "simulator_rtos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

static TimerHandle_t xTimer;

void vTimerCallback( TimerHandle_t xTimer )
{
    Q_UNUSED(xTimer);
    printf("Timer callback\n");
}

void vTask1(void *pvParameters)
{
    Q_UNUSED(pvParameters);
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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    xTaskCreate(vTask1,  
                "Task 1",
                40,
                NULL,
                1,
                NULL);

    /* Starts vTaskStartScheduler() in a parallel thread */
    auto rtos = SimulatorRTOS::instance();
    rtos->start();

    return app.exec();
}
