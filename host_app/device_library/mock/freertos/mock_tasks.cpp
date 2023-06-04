#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <thread>
#include <string>
#include <list>
extern "C"
{
    #include "FreeRTOS.h"
    #include "freertos/task.h"
    #include "portmacro.h"
}
#include <signal.h>

class SimulatedThread
{
public:
    TaskFunction_t taskCode;
    void* parameters;
    TaskHandle_t* createdTask;

    void run() {
        thread_id = pthread_self();
        thread_started = true;
        pthread_setname_np(pthread_self(), _name.c_str());
        taskCode(parameters);
    }

    void start(void)
    {
        worker = std::thread(&SimulatedThread::run, this);
    }

    void stop(void)
    {
        if(thread_started)
        {
            pthread_kill(thread_id, SIGTERM);
            thread_started = false;
        }
        else
        {
            abort();
        }
    }

    void setObjectName(const char *name)
    {
        _name = std::string(name);
    }

    pthread_t thread_id;
    std::atomic<bool> thread_started;
    std::thread worker;
    std::string _name;
};

static std::list<SimulatedThread*> thread_list = std::list<SimulatedThread*>();

portMUX_TYPE global_mux = SPINLOCK_INITIALIZER;

void vTaskStartScheduler( void )
{
    while(true)
    {
        sleep(10);
    };
}

void vTaskDelay( const TickType_t xTicksToDelay )
{
    usleep(pdTICKS_TO_MS(xTicksToDelay)*1000);
}

BaseType_t xTaskCreatePinnedToCore( TaskFunction_t pvTaskCode,
                                   const char * const pcName,
                                   const uint32_t usStackDepth,
                                   void * const pvParameters,
                                   UBaseType_t uxPriority,
                                   TaskHandle_t * const pvCreatedTask,
                                   const BaseType_t xCoreID)
{
    SimulatedThread* thread = new SimulatedThread();
    thread->taskCode = pvTaskCode;
    thread->parameters = pvParameters;
    thread->createdTask = pvCreatedTask;
    thread->setObjectName(pcName);

    thread_list.push_back(thread);

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
        thread->stop();
        delete thread;
    }
}

void terminateAllTasks(void)
{
    thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
                                     [](SimulatedThread* thread)
                                     {
                                         thread->stop();
                                         delete thread;
                                         return true;
                                     }),
                      thread_list.end());
}

TickType_t xTaskGetTickCount( void )
{
    return pdMS_TO_TICKS(esp_timer_get_time()/1000);
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
    printf("vTaskSuspend() not implemented\n");
}

void vTaskSuspendAll( void )
{
    printf("vTaskSuspendAll() not implemented\n");
}

TaskHandle_t xTaskGetCurrentTaskHandle( void )
{
    for(auto thread : thread_list)
    {
        if (thread->thread_id == pthread_self())
        {
            return thread;
        }
    }
    return NULL;
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
