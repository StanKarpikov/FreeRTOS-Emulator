#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <thread>
#include <string>
#include <list>
#include <unistd>
extern "C"
{
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "portmacro.h"
}
#include <signal.h>

struct tskTaskControlBlock
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
        worker = std::thread(&tskTaskControlBlock::run, this);
    }

    void stop(void)
    {
        if(thread_started)
        {
            if (thread_id == pthread_self())
            {
                pthread_exit(0);
            }
            else
            {
                pthread_kill(thread_id, SIGTERM);
            }
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

static std::list<tskTaskControlBlock*> thread_list = std::list<tskTaskControlBlock*>();
static std::list<tskTaskControlBlock*> deleted_thread_list = std::list<tskTaskControlBlock*>();

portMUX_TYPE global_mux = SPINLOCK_INITIALIZER;

void vTaskStartScheduler( void )
{
    while(true)
    {
        sleep(100000);
    };
}

void vTaskDelay( const TickType_t xTicksToDelay )
{
    TickType_t ticks = xTicksToDelay;
    usleep(pdTICKS_TO_MS(ticks)*1000);
}

extern "C" BaseType_t xTaskCreatePinnedToCore( TaskFunction_t pvTaskCode,
                                   const char * const pcName,
                                   const configSTACK_DEPTH_TYPE usStackDepth,
                                   void * const pvParameters,
                                   UBaseType_t uxPriority,
                                   TaskHandle_t * const pvCreatedTask,
                                   const BaseType_t xCoreID)
{
    tskTaskControlBlock* thread = new tskTaskControlBlock();

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

extern "C" TaskHandle_t xTaskCreateStaticPinnedToCore( TaskFunction_t pvTaskCode,
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

extern "C" void vTaskDelete( TaskHandle_t xTaskToDelete )
{
    if(!xTaskToDelete)
    {
        xTaskToDelete = xTaskGetCurrentTaskHandle();
        if(!xTaskToDelete)
        {
            abort();
        }
    }
    tskTaskControlBlock* thread = xTaskToDelete;
    thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
                                     [thread](tskTaskControlBlock* check_thread)
                                     {
                                         return check_thread == thread;
                                     }),
                                     thread_list.end());
    if (thread) {
        deleted_thread_list.push_back(thread);
        thread->stop();
        delete xTaskToDelete; /* TODO: We never reach this step if this is called from the running thread */
    }
}

void terminateAllTasks(void)
{
    thread_list.erase(std::remove_if(thread_list.begin(), thread_list.end(),
                                     [](tskTaskControlBlock* thread)
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

extern "C" eTaskState eTaskGetState( TaskHandle_t xTask )
{
    if (!xTask)
    {
        return eRunning;
    }
    tskTaskControlBlock* thread = xTask;
    for(auto thread_check : deleted_thread_list)
    {
        if (thread_check == thread)
        {
            return eDeleted;
        }
    }
    if (thread->thread_id == pthread_self())
    {
        return eRunning;
    }
    for(auto thread_check : thread_list)
    {
        if (thread_check == thread)
        {
            if(thread->thread_started)
            {
                return eReady;
            }
            else
            {
                return eSuspended;
            }
        }
    }
    return eInvalid;
}
