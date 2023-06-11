# FreeRTOS Emulator (Mock Layer)

The idea of this project is to create a simple mock layer that can be used to compile and run embedded projects in a desktop environment.
Unlike other FreeRTOS emulators, this one **does not** implement task switching using signals and does not set thread priorities.

This imposes some limitations, but makes it possible to run it as a part of another application that needs to use signals (for example, one can implement a supervisor, GUI, or a communication interface to simulate peripheral devices of an embedded system inside the single executable). This makes it more like a mock layer for testing rather than a fully functional FreeRTOS port.

# Requirements

This progect targets FreeRTOS V10.5.0, but other releases should be compatible with minor changes.

The project was originally developed to create a mock layer for an ESP32 project, thus it has ESP32 specific functions like `xTaskCreatePinnedToCore`. It can be used with generic FreeRTOS projects as well, but it may require implementation of additional functions.

## How to Use the Emulator

Refer to the examples in the corresponding folder.
