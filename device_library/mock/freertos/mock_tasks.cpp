extern "C"
{
    #include "FreeRTOS.h"
    #include "freertos/task.h"
    #include "portmacro.h"
}
#include <QThread>
#include <QDateTime>
#include <QList>

class SimulatedThread : public QThread
{
public:
    TaskFunction_t taskCode;
    void* parameters;
    TaskHandle_t* createdTask;

    void run() override {
        taskCode(parameters);
    }
};

static QList<SimulatedThread*> thread_list = QList<SimulatedThread*>();

portMUX_TYPE global_mux = SPINLOCK_INITIALIZER;

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

    SimulatedThread* thread = new SimulatedThread();
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
    TaskHandle_t pvCreatedTask;
    xTaskCreatePinnedToCore(pvTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, &pvCreatedTask, xCoreID);
    return pvCreatedTask;
}

void vTaskDelete( TaskHandle_t xTaskToDelete )
{
    SimulatedThread* thread = static_cast<SimulatedThread*>(xTaskToDelete);

    thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
                                     [thread](SimulatedThread* check_thread)
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
                                     [](SimulatedThread* thread)
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

void vPortEnterCritical( portMUX_TYPE *mux, int timeout )
{

}

void vPortExitCritical( portMUX_TYPE *mux )
{

}

void vTaskSuspend( TaskHandle_t xTaskToSuspend )
{
    /* No action, only used in the event loop after exit */
    qDebug() << "vTaskSuspend() not implemented";
}

void vTaskSuspendAll( void )
{
    qDebug() << "vTaskSuspendAll() not implemented";
}

TaskHandle_t xTaskGetCurrentTaskHandle( void )
{
    return QThread::currentThreadId();
}

TaskHandle_t xTaskGetIdleTaskHandleForCPU( UBaseType_t cpuid )
{
    /* No need to implement */
    return NULL;
}

BaseType_t xTaskGetSchedulerState( void )
{
    return taskSCHEDULER_RUNNING;
}

extern "C" void heap_caps_enable_nonos_stack_heaps(void)
{
    /* No need to implement */
}
