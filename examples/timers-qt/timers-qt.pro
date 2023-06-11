QT += widgets

CONFIG += c++11 ordered depend_includepath

SOURCES += main-timers-qt.cpp

INCLUDEPATH += $$PWD \
               $$PWD/../common \
               $$PWD/../common/FreeRTOS-Kernel/include

include($$PWD/../../freertos-mock-qt/freertos-mock-qt.pri)
