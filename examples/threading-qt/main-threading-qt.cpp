#include <QDebug>
#include <QApplication>
#include "simulator_rtos.h"
#include "FreeRTOS.h"
#include "task.h"

void vTask1(void *pvParameters)
{
    Q_UNUSED(pvParameters);
    printf("Task 1 started\n");
    for (int i=0; i<4; i++)
    {
        printf("Task 1 is running\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    printf("Task 1 stopped\n");
    vTaskDelete(NULL);
}

void vTask2(void *pvParameters)
{
    Q_UNUSED(pvParameters);
    printf("Task 2 started\n");
    for (;;)
    {
        printf("Task 2 is running\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
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

    xTaskCreate(vTask2,
                "Task 2",
                40,
                NULL,
                1,
                NULL);

    auto rtos = SimulatorRTOS::instance();
    rtos->start();

    app.exec();
}
