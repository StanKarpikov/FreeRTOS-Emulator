#include "internal_timer.h"
extern "C"
{
    #include "FreeRTOS.h"
    #include "timers.h"
}

struct tmrTimerControl
{
    InternalTimerHandle timer;
};

extern "C" TimerHandle_t xTimerCreate(const char* const pcTimerName,
                           const TickType_t xTimerPeriodInTicks,
                           const BaseType_t uxAutoReload,
                           void* const pvTimerID,
                           TimerCallbackFunction_t pxCallbackFunction)
{
    tmrTimerControl* tmr_control = new tmrTimerControl;
    tmr_control->timer = InternalTimerHandle(pdTICKS_TO_MS(xTimerPeriodInTicks),
                                                            uxAutoReload == pdFALSE,
                                                            (callback_function_t)pxCallbackFunction,
                                                            nullptr,
                                                            pcTimerName);
    return tmr_control;
}

extern "C" BaseType_t xTimerGenericCommand(TimerHandle_t xTimer,
                                const BaseType_t xCommandID,
                                const TickType_t xOptionalValue,
                                BaseType_t* const pxHigherPriorityTaskWoken,
                                const TickType_t xTicksToWait)
{
    InternalTimerHandle* timerHandle = &xTimer->timer;

    if (timerHandle) {
        switch (xCommandID) {
            case tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR:  // tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR
            case tmrCOMMAND_EXECUTE_CALLBACK:  // tmrCOMMAND_EXECUTE_CALLBACK
                timerHandle->stop();
                if (xCommandID == -1) {
                    if (pxHigherPriorityTaskWoken) {
                        *pxHigherPriorityTaskWoken = pdFALSE;
                    }
                }
                if(timerHandle->pxCallbackFunction)
                {
                    timerHandle->pxCallbackFunction(timerHandle->arg);
                }
                break;
            case tmrCOMMAND_START_DONT_TRACE:  // tmrCOMMAND_START_DONT_TRACE
            case tmrCOMMAND_START:  // tmrCOMMAND_START
                timerHandle->start();
                break;
            case tmrCOMMAND_RESET:  // tmrCOMMAND_RESET
                timerHandle->reset();
                break;
            case tmrCOMMAND_STOP:  // tmrCOMMAND_STOP
                timerHandle->stop();
                break;
            case tmrCOMMAND_CHANGE_PERIOD:  // tmrCOMMAND_CHANGE_PERIOD
                timerHandle->setPeriod(xOptionalValue);
                break;
            case tmrCOMMAND_DELETE:  // tmrCOMMAND_DELETE
                timerHandle->stop();
                delete timerHandle;
                break;
            default:
                break;
        }
    }

    return pdPASS;  // Return success code
}
