/**
 * @file simulator_rtos.h
 * @author Stanislav Karpikov
 * @brief Mock layer for FreeRTOS, global scheduler control
 */

/*--------------------------------------------------------------
                       INCLUDES
--------------------------------------------------------------*/

#ifndef SIMULATOR_RTOS_H
#define SIMULATOR_RTOS_H

#include <QThread>
#include <QTimer>
#include <QQueue>

/*--------------------------------------------------------------
                       PUBLIC TYPES
--------------------------------------------------------------*/

class SimulatorRTOS : public QThread
{
    Q_OBJECT
public:
    SimulatorRTOS();

    static SimulatorRTOS *instance(void)
    {
        static SimulatorRTOS rtos;
        return &rtos;
    }

protected:
    QTimer *_timer;
    void run();
};

#endif // SIMULATOR_RTOS_H
