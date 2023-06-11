set(FREERTOS_MOCK_SOURCES
          mock_tasks.cpp
          mock_queue.cpp
          mock_timers.cpp
          mock_event_groups.cpp
)

add_library(freertos_mock STATIC ${FREERTOS_MOCK_SOURCES})

target_include_directories(
    freertos_mock
    SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/timer
    ${CMAKE_CURRENT_SOURCE_DIR}/timer/cpptime
)
