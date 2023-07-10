/**
 * @file simulator_rtos.cpp
 * @author Stanislav Karpikov
 * @brief Mock layer for FreeRTOS, global scheduler control
 */

/*--------------------------------------------------------------
                       INCLUDES
--------------------------------------------------------------*/

extern "C"
{
    #include "FreeRTOS.h"
    #include "task.h"
}
#include "simulator_rtos.h"

/*--------------------------------------------------------------
                       PUBLIC FUNCTIONS
--------------------------------------------------------------*/

SimulatorRTOS::SimulatorRTOS()
{
    setObjectName("[Sim] Device");
}

void SimulatorRTOS::run()
{
    setbuf(stdout, NULL);
    vTaskStartScheduler();
}
