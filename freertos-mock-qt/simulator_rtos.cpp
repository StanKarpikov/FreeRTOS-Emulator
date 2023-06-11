extern "C" {
    #include "FreeRTOS.h"
    #include "task.h"
}
#include "simulator_rtos.h"

SimulatorRTOS::SimulatorRTOS()
{
    setObjectName("[Sim] Device");
}

void SimulatorRTOS::run()
{
    setbuf(stdout, NULL);
    vTaskStartScheduler();
}
