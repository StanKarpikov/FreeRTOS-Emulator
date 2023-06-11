#include <QDebug>
#include <QApplication>
#include <QtWidgets>
#include "simulator_rtos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

static TimerHandle_t xTimer;
static QueueHandle_t xQueue;

typedef struct
{
    char message[255];
} message_t;

void vTimerCallback( TimerHandle_t xTimer )
{
    Q_UNUSED(xTimer);
    BaseType_t pxHigherPriorityTaskWoken;
    message_t element =
    {
       "Timer callback"
    };
    xQueueSendFromISR(
        xQueue,
        &element,
        &pxHigherPriorityTaskWoken
    );
}

void vTask1(void *pvParameters)
{
    QTextEdit* text_edit = static_cast<QTextEdit*>(pvParameters);

    xQueue = xQueueCreate( 10, sizeof( message_t ) );

    xTimer = xTimerCreate("Timer",
                          pdMS_TO_TICKS(2500),
                          pdTRUE,
                          (void*)0,
                          vTimerCallback);

    xTimerStart( xTimer, 0);
    message_t element;

    while(1)
    {
        if( xQueueReceive( xQueue,
                          &( element ),
                          pdMS_TO_TICKS(1000) ) == pdPASS )
        {
            QMetaObject::invokeMethod(text_edit, "append",
                                      Q_ARG(QString, "Received:" + QString(element.message)));
        }
        else
        {
            QMetaObject::invokeMethod(text_edit, "append",
                                      Q_ARG(QString, "Queue is empty"));
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto text_edit = QTextEdit();
    text_edit.resize(320, 240);
    text_edit.show();

    xTaskCreate(vTask1,
                "Task 1",
                40,
                &text_edit,
                1,
                NULL);

    /* Starts vTaskStartScheduler() in a parallel thread */
    auto rtos = SimulatorRTOS::instance();
    rtos->start();

    return app.exec();
}
