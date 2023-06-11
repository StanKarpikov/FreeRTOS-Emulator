#ifndef SIMULATOR_RTOS_H
#define SIMULATOR_RTOS_H

#include <QThread>
#include <QTimer>
#include <QQueue>

class SimulatorRTOS : public QThread
{
    Q_OBJECT
public:
    SimulatorRTOS();

    static SimulatorRTOS* instance(void)
    {
        static SimulatorRTOS rtos;
        return &rtos;
    }

protected:
    QTimer *_timer;
    void run();
};

extern SimulatorRTOS rtos;

#endif // SIMULATOR_RTOS_H
