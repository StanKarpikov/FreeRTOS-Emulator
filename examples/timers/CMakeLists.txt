cmake_minimum_required(VERSION 3.12)
project(example-timers)

set(CMAKE_CXX_STANDARD 11)

set(INCLUDE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/FreeRTOS-Kernel/include
)

set(SOURCES
    main-timers.c
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../freertos-mock" freertos-mock)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}
    freertos_mock
)

target_include_directories(freertos_mock 
                           PRIVATE 
                           ${INCLUDE_PATHS})

target_include_directories(${PROJECT_NAME} 
                           PRIVATE 
                           ${INCLUDE_PATHS})

target_link_libraries(${PROJECT_NAME} pthread m stdc++)
