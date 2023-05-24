#include "microcontrollerenvironment.h"
#include <QVector>
#include <QThread>
#include <QTimer>
#include <QtAlgorithms>
#include <QDebug>
#include "pthread_pause.h"
#include <QCoreApplication>
#include "simulatedtask.h"
#include "portmacro.h"

#define log printf

#define UC_RESPONSIVENESS  (portTICK_PERIOD_MS / 10)

/* Pointer to the TCB of the currently executing task. */
extern tskTCB *pxCurrentTCB;

MicroControllerEnvironment::MicroControllerEnvironment(QObject *parent)
    : QObject(parent),
      m_pTaskList(new QVector<SimulatedTask*>()),
      m_taskTimer(new TaskTimer(*this)),
      m_irqFlags(0),
      m_processorRunning(false)
{
    QThread *thread = new QThread();
    thread->setObjectName("[Sim] Tick timer");
    m_taskTimer->moveToThread(thread);
    thread->start(THREAD_SV_TIMER_PRIO);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
}

SimulatedTask* MicroControllerEnvironment::addTask(TaskFunction_t pxCode, void *pvParameters) {
    QThread* thread = new QThread();
    SimulatedTask * newTask = new SimulatedTask(pxCode, pvParameters);

    m_pTaskList->append(newTask);

    newTask->moveToThread(thread);
    connect(thread, &QThread::started, newTask, &SimulatedTask::run); //, Qt::BlockingQueuedConnection

    qDebug() << "Add new simulated task: "<< newTask << " address @"<< (void*)newTask;
    qDebug() << newTask->thread();

    return newTask; // we save this on the FreeRTOS stack for the task
}

void MicroControllerEnvironment::removeTask(SimulatedTask* task_to_remove)
{
    pthread_pause_disable();
    interruptEventMutex.lock();
//    qDebug() << "Before remove:";
//    foreach(SimulatedTask* task, *m_pTaskList)
//    {
//        qDebug() << task->thread()->objectName().toStdString().c_str();
//    }
    m_pTaskList->erase(std::remove_if(m_pTaskList->begin(), m_pTaskList->end(),
                                      [task_to_remove](SimulatedTask* i) { return i==task_to_remove; }),
                                      m_pTaskList->end());
//    qDebug() << "After remove:";
//    foreach(SimulatedTask* task, *m_pTaskList)
//    {
//        qDebug() << task->thread()->objectName().toStdString().c_str();
//    }
    pthread_pause_enable();
    interruptEventMutex.unlock();
}

void MicroControllerEnvironment::AppendFreeRtosNameToLastTask(char const * const freeRtosTaskName)
{
    SimulatedTask * const lastTask = m_pTaskList->last();
    lastTask->setObjectName(lastTask->objectName().append(freeRtosTaskName));
    lastTask->thread()->setObjectName(QString("Thread of simulated FreeRTOS task: %1").arg(freeRtosTaskName));
}

void MicroControllerEnvironment::run()
{
    pthread_pause_enable(); //Will get inherited by all threads from now on
    //you need to call pthread_pause_enable (or disable) before creating threads,
    //otherwise first (un)pause signal will kill whole process

//        pthread_t this_thread = pthread_self();
//        struct sched_param params;
//        params.sched_priority = 32;
//        pthread_setschedparam(this_thread, SCHED_RR, &params);
//        Qt::HANDLE x = QThread::currentThreadId();

        qDebug() << "Initial Tasks:";
        foreach(SimulatedTask* task, *m_pTaskList)
        {
            qDebug() << task->thread()->objectName();
        }
    
        /* Create a pending tick to ensure the first task is started as soon as
        this thread pends. */
        m_irqFlags.setFlag(InterruptFlags::TaskTimer);

        m_processorRunning = true;

        Q_ASSERT(m_taskTimer->thread()->isRunning());
        QMetaObject::invokeMethod(m_taskTimer, "startTimer", Qt::BlockingQueuedConnection);

        forever
        {
            interruptEventMutex.lock();
//            vPortEnterCritical();

            /* Used to indicate whether the simulated interrupt processing has
            necessitated a context switch to another task/thread. */
            bool freertosSchedulerSwitchRequired = false;

            /* For each interrupt we are interested in processing, each of which is
            represented by a bit in the 32bit ulPendingInterrupts variable. */
            for(int i = 0; i < m_irqFlags.numFlags(); i++ )
            {
                /* Is the simulated interrupt pending? */
                if( m_irqFlags.isFlagSet(InterruptFlags::Interrupt(1UL << i)))
                {
                    freertosSchedulerSwitchRequired = true;
                }
            }
            
            m_irqFlags.clear();

            if( freertosSchedulerSwitchRequired)
            {
//                log("> Switch context IN\n");
                void *pvOldCurrentTCB;

                pvOldCurrentTCB = pxCurrentTCB;

                /* Select the next task to run. */
                vTaskSwitchContext();

                /* If the task selected to enter the running state is not the task
                that is already in the running state. */
                if( pvOldCurrentTCB != pxCurrentTCB )
                {
                    /* Suspend the old thread. */
                    {
                        tskTCB* old_tcb = (tskTCB*)pvOldCurrentTCB;
                        xThreadState *old_state = (xThreadState*)(old_tcb->pxTopOfStack);
                        SimulatedTask *oldTask = old_state->pvThread;
                        if(oldTask)
                        {
                            auto thread = oldTask->thread();
                            if (oldTask->thread()->isRunning())
                            {
                                log("# Set idle %s\n", oldTask->thread()->objectName().toStdString().c_str());
                                oldTask->paused = false;
                                pthread_pause((pthread_t)oldTask->thread_id);
        //                        thread->setPriority(THREAD_TASK_IDLE_PRIO);
        //                        struct sched_param params;
        //                        params.sched_priority = 1;
        //                        pthread_setschedparam((pthread_t)oldTask->thread_id, SCHED_RR, &params);
                            }
                            else
                            {
                                log("# Not running %s???\n", oldTask->thread()->objectName().toStdString().c_str());
                            }
                        }
                    }

                    /* Obtain the state of the task now selected to enter the
                    Running state. */
                    {
                        tskTCB* new_tcb = NULL;
                        xThreadState *new_state = NULL;
                        new_tcb = (tskTCB*)pxCurrentTCB;
                        new_state = (xThreadState*)(new_tcb->pxTopOfStack);
                        SimulatedTask *newTask = new_state->pvThread;
                        if (newTask->thread()->isRunning())
                        {
                            log("# Set running %s\n", newTask->thread()->objectName().toStdString().c_str());
                            newTask->paused = false;
                            pthread_unpause((pthread_t)newTask->thread_id);
    //                        newTask->thread()->setPriority(THREAD_TASK_RUNNING_PRIO);
    //                        struct sched_param params;
    //                        params.sched_priority = 31;
    //                        pthread_setschedparam((pthread_t)newTask->thread_id, SCHED_RR, &params);
                        }
                        else
                        {
                            newTask->thread()->setObjectName(new_tcb->pcTaskName);
                            log("# Start %s\n", new_tcb->pcTaskName);
                            newTask->paused = false;
                            newTask->thread()->start();
    //                        do
    //                        {
    //                            QThread::msleep(20);
    //                            log("Wait %s running...\n", new_tcb->pcTaskName);
    //                        } while(newTask->thread_id == 0);

    //                        struct sched_param params;
    //                        params.sched_priority = 31;
    //                        pthread_setschedparam((pthread_t)newTask->thread_id, SCHED_RR, &params);
    //
    //                        QMetaObject::invokeMethod(newTask, "run", Qt::BlockingQueuedConnection);
                        }
                    }
                }
                else
                {
                    tskTCB* old_tcb = (tskTCB*)pvOldCurrentTCB;
                    xThreadState *old_state = (xThreadState*)(old_tcb->pxTopOfStack);
                    SimulatedTask *oldTask = old_state->pvThread;
                    auto thread = oldTask->thread();
                    if(strcmp(thread->objectName().toStdString().c_str(), "IDLE") != 0){
                        log("# -- No change continue %s\n", thread->objectName().toStdString().c_str());
                    }
                }
//                log("< Switch context OUT\n");
            }

//            vPortExitCritical();
            interruptEventMutex.unlock();
            QThread::currentThread()->msleep(UC_RESPONSIVENESS);
//            QCoreApplication::processEvents();
            while(m_irqFlags.isZero())
            {
                QThread::currentThread()->msleep(UC_RESPONSIVENESS);
//                QCoreApplication::processEvents();
            }
        }
}

void MicroControllerEnvironment::yield() {
    Q_ASSERT(m_processorRunning);
    m_irqFlags.setFlag(InterruptFlags::Yield);
    foreach(SimulatedTask *task, *m_pTaskList)
    {
        if(task && task->thread()->isRunning())
        {
//            task->thread()->setPriority(THREAD_TASK_IDLE_PRIO);
            log("Yield %s\n", task->thread()->objectName().toStdString().c_str());
//            pthread_pause((pthread_t)task->thread_id);
//            task->paused = true;
            pthread_pause_yield();
//            pthread_pause((pthread_t)task->thread_id);
//            pthread_pause_yield();
            log("Restored %s\n", task->thread()->objectName().toStdString().c_str());
        }
        if (task->thread() == QThread::currentThread()) {
            QThread::currentThread()->usleep(UC_RESPONSIVENESS);
        }
    }
}



MicroControllerEnvironment::~MicroControllerEnvironment()
{
    // Delete all tasks
    qDeleteAll(m_pTaskList->begin(), m_pTaskList->end());
    // Remove tasks from list
    m_pTaskList->clear();

    // Remove list
    delete m_pTaskList;

}

