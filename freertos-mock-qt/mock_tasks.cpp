/**
 * @file mock_tasks.cpp
 * @author Stanislav Karpikov
 * @brief Mock layer for FreeRTOS tasks file (Qt version)
 */

/*--------------------------------------------------------------
                       INCLUDES
--------------------------------------------------------------*/

extern "C"
{
    #include "FreeRTOS.h"
    #include "task.h"
    #include "portmacro.h"
}
#include <QThread>
#include <QDateTime>
#include <QList>

/*--------------------------------------------------------------
                       PRIVATE TYPES
--------------------------------------------------------------*/

struct tskTaskControlBlock : public QThread
{
public:
    TaskFunction_t taskCode;
    void* parameters;
    TaskHandle_t* createdTask;

    void run() override {
        taskCode(parameters);
    }
};

/*--------------------------------------------------------------
                       PRIVATE DATA
--------------------------------------------------------------*/

static QList<tskTaskControlBlock*> thread_list = QList<tskTaskControlBlock*>();

#if ESP_PLATFORM
portMUX_TYPE global_mux = SPINLOCK_INITIALIZER;
#endif

/*--------------------------------------------------------------
                       PUBLIC FUNCTIONS
--------------------------------------------------------------*/

void vTaskStartScheduler( void )
{
    while(true)
    {
        QThread::sleep(10);
    };
}

void vTaskDelay( const TickType_t xTicksToDelay )
{
    QThread::msleep(pdTICKS_TO_MS(xTicksToDelay));
}

BaseType_t xTaskCreatePinnedToCore( TaskFunction_t pvTaskCode,
                                   const char * const pcName,
                                   const uint32_t usStackDepth,
                                   void * const pvParameters,
                                   UBaseType_t uxPriority,
                                   TaskHandle_t * const pvCreatedTask,
                                   const BaseType_t xCoreID)
{
    Q_UNUSED(usStackDepth);
    Q_UNUSED(uxPriority);
    Q_UNUSED(xCoreID);

    tskTaskControlBlock* thread = new tskTaskControlBlock();
    thread->taskCode = pvTaskCode;
    thread->parameters = pvParameters;
    thread->createdTask = pvCreatedTask;
    thread->setObjectName(pcName);

    thread_list.append(thread);

    thread->start();
    if(pvCreatedTask)
    {
        *pvCreatedTask = thread;
    }

    return pdPASS;
}

TaskHandle_t xTaskCreateStaticPinnedToCore( TaskFunction_t pvTaskCode,
                                           const char * const pcName,
                                           const uint32_t ulStackDepth,
                                           void * const pvParameters,
                                           UBaseType_t uxPriority,
                                           StackType_t * const pxStackBuffer,
                                           StaticTask_t * const pxTaskBuffer,
                                           const BaseType_t xCoreID )
{
    Q_UNUSED(pxStackBuffer);
    Q_UNUSED(pxTaskBuffer);
    TaskHandle_t pvCreatedTask;
    xTaskCreatePinnedToCore(pvTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, &pvCreatedTask, xCoreID);
    return pvCreatedTask;
}

extern "C" BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                                  const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                  const configSTACK_DEPTH_TYPE usStackDepth,
                                  void * const pvParameters,
                                  UBaseType_t uxPriority,
                                  TaskHandle_t * const pxCreatedTask )
{
    return xTaskCreatePinnedToCore(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask, 0);
}

void vTaskDelete( TaskHandle_t xTaskToDelete )
{
    tskTaskControlBlock* thread = xTaskToDelete;

    thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
                                     [thread](tskTaskControlBlock* check_thread)
                                     {
                                         return check_thread == thread;
                                     }),
                                     thread_list.end());
    if (thread) {
        thread->quit();
        thread->wait();
        delete thread;
    }
}

void terminateAllTasks(void)
{
    thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
                                     [](tskTaskControlBlock* thread)
                                     {
                                         thread->quit();
                                         delete thread;
                                         return true;
                                     }),
                      thread_list.end());
}

TickType_t xTaskGetTickCount( void )
{
    static uint64_t start_tick = 0;
    if(!start_tick)
    {
        start_tick = QDateTime::currentSecsSinceEpoch();
    }
    return pdMS_TO_TICKS(QDateTime::currentSecsSinceEpoch() - start_tick);
}

TickType_t xTaskGetTickCountFromISR( void )
{
    return xTaskGetTickCount();
}

extern "C" void vTaskSuspend( TaskHandle_t xTaskToSuspend )
{
    Q_UNUSED(xTaskToSuspend);
    qDebug() << "vTaskSuspend() not implemented, see stdlib example in the freertos-mock folder";

}

extern "C" void vTaskResume( TaskHandle_t xTaskToResume )
{
    Q_UNUSED(xTaskToResume);
    qDebug() << "vTaskResume() not implemented, see stdlib example in the freertos-mock folder";
}

extern "C" void vTaskSuspendAll( void )
{
    qDebug() << "vTaskSuspendAll() not implemented, see stdlib example in the freertos-mock folder";
}

TaskHandle_t xTaskGetCurrentTaskHandle( void )
{
    return (tskTaskControlBlock*)QThread::currentThreadId();
}

TaskHandle_t xTaskGetIdleTaskHandleForCPU( UBaseType_t cpuid )
{
    Q_UNUSED(cpuid);
    /* No need to implement */
    return NULL;
}

BaseType_t xTaskGetSchedulerState( void )
{
    return taskSCHEDULER_RUNNING;
}
