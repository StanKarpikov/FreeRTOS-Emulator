#include "simulator_rtos.h"
extern "C"
{
    #include "FreeRTOS.h"
    #include "timers.h"
}
#include <QTimer>
#include <QApplication>

struct tmrTimerControl
{
    QTimer timer;
};

extern "C" TimerHandle_t xTimerCreate(const char* const pcTimerName,
                           const TickType_t xTimerPeriodInTicks,
                           const BaseType_t uxAutoReload,
                           void* const pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction)
{
    Q_UNUSED(pcTimerName);
    Q_UNUSED(pvTimerID);

    tmrTimerControl* timer = new tmrTimerControl();
    timer->timer.moveToThread(SimulatorRTOS::instance()->thread());
    QObject::connect(&timer->timer, &QTimer::timeout, [pxCallbackFunction]() {
        if (pxCallbackFunction) {
            pxCallbackFunction(nullptr);
        }
    });

    timer->timer.setInterval(pdTICKS_TO_MS(xTimerPeriodInTicks));
    if(uxAutoReload == pdFALSE)
    {
        timer->timer.setSingleShot(true);
    }
    return timer;
}

extern "C" BaseType_t xTimerGenericCommand(TimerHandle_t xTimer,
                                const BaseType_t xCommandID,
                                const TickType_t xOptionalValue,
                                BaseType_t* const pxHigherPriorityTaskWoken,
                                const TickType_t xTicksToWait) {
    Q_UNUSED(pxHigherPriorityTaskWoken);
    Q_UNUSED(xTicksToWait);

    tmrTimerControl* timerHandle = xTimer;

    if (timerHandle) {
        switch (xCommandID) {
        case tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR:  // tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR
        case tmrCOMMAND_EXECUTE_CALLBACK:  // tmrCOMMAND_EXECUTE_CALLBACK
            if (timerHandle->timer.isActive()) {
                timerHandle->timer.stop();
                if (xCommandID == -1) {
                    if (pxHigherPriorityTaskWoken) {
                        *pxHigherPriorityTaskWoken = pdFALSE;
                    }
                }
                QMetaObject::invokeMethod(&timerHandle->timer, "timeout", Qt::QueuedConnection);
            }
            break;
        case tmrCOMMAND_START_DONT_TRACE:  // tmrCOMMAND_START_DONT_TRACE
            QMetaObject::invokeMethod(&timerHandle->timer, "start", Qt::QueuedConnection);
            QApplication::processEvents();
            break;
        case tmrCOMMAND_START:  // tmrCOMMAND_START
            QMetaObject::invokeMethod(&timerHandle->timer, "start", Qt::QueuedConnection, Q_ARG(int, timerHandle->timer.interval()));
            QApplication::processEvents();
            break;
        case tmrCOMMAND_RESET:  // tmrCOMMAND_RESET
            QMetaObject::invokeMethod(&timerHandle->timer, "stop", Qt::QueuedConnection);
            QMetaObject::invokeMethod(&timerHandle->timer, "start", Qt::QueuedConnection, Q_ARG(int, timerHandle->timer.interval()));
            QApplication::processEvents();
            break;
        case tmrCOMMAND_STOP:  // tmrCOMMAND_STOP
            QMetaObject::invokeMethod(&timerHandle->timer, "stop", Qt::QueuedConnection);
            QApplication::processEvents();
            break;
        case tmrCOMMAND_CHANGE_PERIOD:  // tmrCOMMAND_CHANGE_PERIOD
            timerHandle->timer.setInterval(pdTICKS_TO_MS(xOptionalValue));
            break;
        case tmrCOMMAND_DELETE:  // tmrCOMMAND_DELETE
            delete timerHandle;
            break;
        default:
            break;
        }
    }

    return pdPASS;  // Return success code
}
