SOURCES += $$PWD/mock_event_groups.cpp \
           $$PWD/mock_queue.cpp \
           $$PWD/mock_tasks.cpp \
           $$PWD/mock_timers.cpp \
           $$PWD/simulator_rtos.cpp

INCLUDEPATH += $$PWD

HEADERS += $$PWD/portmacro.h \
           $$PWD/ringbuf.h \
           $$PWD/simulator_rtos.h
