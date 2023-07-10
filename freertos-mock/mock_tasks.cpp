/**
 * @file mock_tasks.cpp
 * @author Stanislav Karpikov
 * @brief Mock layer for FreeRTOS tasks file
 */

/*--------------------------------------------------------------
                       INCLUDES
--------------------------------------------------------------*/

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <list>
#include <unistd.h>
extern "C"
{
    #include "FreeRTOS.h"
    #include "task.h"
    #include "portmacro.h"
}
#include <signal.h>

/*--------------------------------------------------------------
                       PRIVATE DEFINES
--------------------------------------------------------------*/

/** Maximum waiting time to get the task into suspended state */
#define WAIT_FOR_TASK_SUSPENDED_TIMEOUT 3000

/*--------------------------------------------------------------
                       PRIVATE TYPES
--------------------------------------------------------------*/

typedef std::chrono::duration<int, std::milli> milliseconds_type;

#if ESP_PLATFORM
portMUX_TYPE global_mux = SPINLOCK_INITIALIZER;
#endif

struct tskTaskControlBlock
{
public:
    void run()
    {
        thread_id = pthread_self();
        thread_started = true;
        pthread_setname_np(pthread_self(), _name.c_str());
        taskCode(parameters);
    }

    void start(void)
    {
        suspend_requested.unlock();
        worker = std::thread(&tskTaskControlBlock::run, this);
    }

    void process_events(void)
    {
        if(!suspend_requested.try_lock())
        {
            task_suspended.notify_all();
            suspend_requested.lock();
        }
        suspend_requested.unlock();

        if(!delete_requested.try_lock())
        {
            stop();
        }
        delete_requested.unlock();
    }

    void suspend(void)
    {
        if(thread_suspended)
        {
            return;
        }
        thread_suspended = true;
        if(suspend_requested.try_lock())
        {
            std::unique_lock<std::mutex> lock_suspended(task_suspended_mutex);
            milliseconds_type duration(WAIT_FOR_TASK_SUSPENDED_TIMEOUT);
            if(task_suspended.wait_for(lock_suspended, duration) == std::cv_status::timeout)
            {
                std::cout << "Error waiting for task suspention";
            }
        }
    }

    void resume(void)
    {
        thread_suspended = false;
        suspend_requested.unlock();
    }

    void exit(void)
    {
        pthread_exit(0);
    }

    void stop(void)
    {
        if(thread_started)
        {
            if (thread_id == pthread_self())
            {
                exit();
            }
            else
            {
                delete_requested.lock();
                worker.join();
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

    TaskFunction_t taskCode;
    void* parameters;
    TaskHandle_t* createdTask;
    std::condition_variable task_suspended;
    std::mutex task_suspended_mutex;

    pthread_t thread_id;
    std::atomic<bool> thread_started;
    std::atomic<bool> thread_suspended;
    std::thread worker;
    std::string _name;
    std::mutex suspend_requested;
    std::mutex delete_requested;
};

/*--------------------------------------------------------------
                       PRIVATE DATA
--------------------------------------------------------------*/

static std::condition_variable tasks_deleted;
static std::condition_variable request_task_deletion;
static std::mutex task_deletion_mutex;
static std::mutex task_management_mutex;
static std::list<tskTaskControlBlock*> thread_list = std::list<tskTaskControlBlock*>();
static std::list<tskTaskControlBlock*> deleted_thread_list = std::list<tskTaskControlBlock*>();

/*--------------------------------------------------------------
                      PUBLIC FUNCTIONS
--------------------------------------------------------------*/

extern "C" void vTaskStartScheduler( void )
{
    std::list<tskTaskControlBlock*> deleted_thread_list_copy;
    std::list<tskTaskControlBlock*> thread_list_copy;
    while(true)
    {
        std::unique_lock<std::mutex> lock_del(task_deletion_mutex);
        request_task_deletion.wait(lock_del);
        {
            std::unique_lock<std::mutex> lock_man(task_management_mutex);
            deleted_thread_list_copy = deleted_thread_list;
            thread_list_copy = thread_list;
        }
        for(auto thread : deleted_thread_list_copy)
        {
            bool found = (std::find(thread_list_copy.begin(), thread_list_copy.end(), thread) != thread_list_copy.end());
            if(found)
            {
                thread->stop();
                delete thread;
                std::unique_lock<std::mutex> lock_man(task_management_mutex);
                thread_list.remove(thread);
            }
        }
        tasks_deleted.notify_one();
    };
}

extern "C" void vTaskDelay( const TickType_t xTicksToDelay )
{
    tskTaskControlBlock *task = xTaskGetCurrentTaskHandle();
    task->process_events();
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

    {
        std::unique_lock<std::mutex> lk(task_management_mutex);
        thread_list.push_back(thread);
    }

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

extern "C" BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                            const configSTACK_DEPTH_TYPE usStackDepth,
                            void * const pvParameters,
                            UBaseType_t uxPriority,
                            TaskHandle_t * const pxCreatedTask )
{
    return xTaskCreatePinnedToCore(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask, 0);
}

extern "C" void vTaskDelete( TaskHandle_t xTaskToDelete )
{
    bool delete_self = false;
    if(!xTaskToDelete)
    {
        xTaskToDelete = xTaskGetCurrentTaskHandle();
        if(!xTaskToDelete)
        {
            abort();
        }
        delete_self = true;
    }
    {
        std::unique_lock<std::mutex> lk(task_management_mutex);
        deleted_thread_list.push_back(xTaskToDelete);
        request_task_deletion.notify_one();
    }
    if(delete_self)
    {
        while(1)
        {
            /* Wait until deleted */
            xTaskToDelete->process_events();
        }
    }
    else
    {
        std::unique_lock<std::mutex> lk(task_management_mutex);
        tasks_deleted.wait(lk);
    }
}

extern "C" void terminateAllTasks(void)
{
    std::unique_lock<std::mutex> lk(task_management_mutex);
    for(auto thread : thread_list)
    {
        deleted_thread_list.push_back(thread);
    }
    request_task_deletion.notify_one();
}

extern "C" TickType_t xTaskGetTickCount( void )
{
    return pdMS_TO_TICKS(port_get_time_ms());
}

extern "C" TickType_t xTaskGetTickCountFromISR( void )
{
    return xTaskGetTickCount();
}

extern "C" void vTaskSuspend( TaskHandle_t xTaskToSuspend )
{
    /* This will only suspend the task on the next sleep call */
    xTaskToSuspend->suspend();
}

extern "C" void vTaskResume( TaskHandle_t xTaskToResume )
{
    xTaskToResume->resume();
}

extern "C" void vTaskSuspendAll( void )
{
    std::unique_lock<std::mutex> lk(task_management_mutex);
    for(auto thread : thread_list)
    {
        if (thread->thread_id == pthread_self())
        {
            thread->suspend();
        }
    }
}

extern "C" TaskHandle_t xTaskGetCurrentTaskHandle( void )
{
    std::unique_lock<std::mutex> lk(task_management_mutex);
    for(auto thread : thread_list)
    {
        if (thread->thread_id == pthread_self())
        {
            return thread;
        }
    }
    return NULL;
}

extern "C" TaskHandle_t xTaskGetIdleTaskHandleForCPU( UBaseType_t cpuid )
{
    /* No need to implement */
    return NULL;
}

extern "C" BaseType_t xTaskGetSchedulerState( void )
{
    return taskSCHEDULER_RUNNING;
}

extern "C" eTaskState eTaskGetState( TaskHandle_t xTask )
{
    std::unique_lock<std::mutex> lk(task_management_mutex);
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
            if(thread->thread_suspended)
            {
                return eSuspended;
            }
            if(thread->thread_started)
            {
                return eReady;
            }
            break;
        }
    }
    return eInvalid;
}

extern "C" UBaseType_t uxTaskGetNumberOfTasks(void)
{
    return thread_list.size();
}

extern "C" UBaseType_t uxTaskGetSystemState( TaskStatus_t * const pxTaskStatusArray,
                                 const UBaseType_t uxArraySize,
                                 uint32_t * const pulTotalRunTime )

{
    /* TODO: This mutex can cause a dead lock because of eTaskGetState() */
    /* std::unique_lock<std::mutex> lk(task_management_mutex); */
    int i=0;
    for(auto thread_check : thread_list)
    {
        pxTaskStatusArray[i].pcTaskName = thread_check->_name.c_str();
        pxTaskStatusArray[i].xTaskNumber = i;
        pxTaskStatusArray[i].eCurrentState = eTaskGetState(thread_check);
#if ESP_PLATFORM
        pxTaskStatusArray[i].xCoreID = 0;
#endif
        pxTaskStatusArray[i].usStackHighWaterMark = 0;
        i++;
    }
    return thread_list.size();
}
