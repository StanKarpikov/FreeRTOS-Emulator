#include "simulator_rtos.h"
extern "C"
{
    #include "FreeRTOS.h"
    #include "freertos/timers.h"
}
#include <QTimer>
#include <QApplication>

TimerHandle_t xTimerCreate(const char* const pcTimerName,
                           const TickType_t xTimerPeriodInTicks,
                           const UBaseType_t uxAutoReload,
                           void* const pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction) {
    Q_UNUSED(pcTimerName);
    Q_UNUSED(pvTimerID);

    QTimer* timer = new QTimer();
    timer->moveToThread(rtos.thread());
    QObject::connect(timer, &QTimer::timeout, [pxCallbackFunction]() {
        if (pxCallbackFunction) {
            pxCallbackFunction(nullptr);
        }
    });

    timer->setInterval(pdTICKS_TO_MS(xTimerPeriodInTicks));
    if(uxAutoReload == pdFALSE)
    {
        timer->setSingleShot(true);
    }
    return timer;
}

BaseType_t xTimerGenericCommand(TimerHandle_t xTimer,
                                const BaseType_t xCommandID,
                                const TickType_t xOptionalValue,
                                BaseType_t* const pxHigherPriorityTaskWoken,
                                const TickType_t xTicksToWait) {
    Q_UNUSED(pxHigherPriorityTaskWoken);
    Q_UNUSED(xTicksToWait);

    QTimer* timerHandle = static_cast<QTimer*>(xTimer);

    if (timerHandle) {
        switch (xCommandID) {
        case tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR:  // tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR
        case tmrCOMMAND_EXECUTE_CALLBACK:  // tmrCOMMAND_EXECUTE_CALLBACK
            if (timerHandle->isActive()) {
                timerHandle->stop();
                if (xCommandID == -1) {
                    if (pxHigherPriorityTaskWoken) {
                        *pxHigherPriorityTaskWoken = pdFALSE;
                    }
                }
                QMetaObject::invokeMethod(timerHandle, "timeout", Qt::QueuedConnection);
            }
            break;
        case tmrCOMMAND_START_DONT_TRACE:  // tmrCOMMAND_START_DONT_TRACE
            QMetaObject::invokeMethod(timerHandle, "start", Qt::QueuedConnection);
            QApplication::processEvents();
            break;
        case tmrCOMMAND_START:  // tmrCOMMAND_START
            QMetaObject::invokeMethod(timerHandle, "start", Qt::QueuedConnection, Q_ARG(int, timerHandle->interval()));
            QApplication::processEvents();
            break;
        case tmrCOMMAND_RESET:  // tmrCOMMAND_RESET
            QMetaObject::invokeMethod(timerHandle, "stop", Qt::QueuedConnection);
            QMetaObject::invokeMethod(timerHandle, "start", Qt::QueuedConnection, Q_ARG(int, timerHandle->interval()));
            QApplication::processEvents();
            break;
        case tmrCOMMAND_STOP:  // tmrCOMMAND_STOP
            QMetaObject::invokeMethod(timerHandle, "stop", Qt::QueuedConnection);
            QApplication::processEvents();
            break;
        case tmrCOMMAND_CHANGE_PERIOD:  // tmrCOMMAND_CHANGE_PERIOD
            timerHandle->setInterval(pdTICKS_TO_MS(xOptionalValue));
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
