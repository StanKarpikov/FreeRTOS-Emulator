cmake_minimum_required(VERSION 3.12)
project(example-threading)

set(CMAKE_CXX_STANDARD 11)

set(INCLUDE_PATHS
    ${ESP_IDF_COMPONENTS}/freertos/FreeRTOS-Kernel/include
    ${ESP_IDF_COMPONENTS}/freertos/FreeRTOS-Kernel/include/freertos
)

set(SOURCES
    main.c
)

add_subdirectory("../freertos_mock")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}
    freertos_mock
)

target_link_libraries(${PROJECT_NAME} pthread m stdc++)
