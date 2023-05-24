#include "simulatedtask.h"

uint_least16_t SimulatedTask::s_totalNumThreads = 0;

void SimulatedTask::run()
{
    thread_id = QThread::currentThreadId();
    m_pxCode(m_pvParameters);
}
