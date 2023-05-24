#ifndef SIMULATEDTASK_H
#define SIMULATEDTASK_H

#include <QObject>
#include <QString>
#include <QThread>

#include "FreeRTOS.h"

class SimulatedTask : public QObject
{
Q_OBJECT

public:
    SimulatedTask(TaskFunction_t pxCode, void *pvParameters)
        :m_pxCode(pxCode),
         m_pvParameters(pvParameters)
    {setObjectName(QString("Simulated FreeRTOS task (%1): ").arg(s_totalNumThreads++));}

    Qt::HANDLE thread_id = 0;
    QAtomicInt paused = false;

public slots:
    void run();

private:
    const TaskFunction_t m_pxCode;
    void * const m_pvParameters;

    static uint_least16_t s_totalNumThreads;
};

/* The WIN32 simulator runs each task in a thread.  The context switching is
managed by the threads, so the task stack does not have to be managed directly,
although the task stack is still used to hold an xThreadState structure this is
the only thing it will ever hold.  The structure indirectly maps the task handle
to a thread handle. */
typedef struct
{
    /* Handle of the thread that executes the task. */
    SimulatedTask *pvThread;

} xThreadState;

#endif // SIMULATEDTASK_H
