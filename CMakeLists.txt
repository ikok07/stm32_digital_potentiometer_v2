cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32_digital_potentiometer_v2)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add sources to executable
file(GLOB APP_SOURCES
        "Src/*.c"
        "Src/msp/*.c"
        "./startup_stm32l031xx.s"
)
file (GLOB CMSIS_SOURCES "lib/CMSIS/Device/ST/STM32L0xx/Source/*.c")
file (GLOB HAL_SOURCES "lib/HAL/Src/*.c")
file (GLOB FREERTOS_SOURCES
        "lib/FreeRTOS/Src/*.c"
        "lib/FreeRTOS_Port/Src/*.c"
)
file (GLOB DAC_X050X_SOURCES "lib/DAC_X050X/Src/*.c")
file (GLOB SHVAL_SOURCES "lib/shared_values/Src/*.c")
file (GLOB TASKS_SCHEDULER_SOURCES "lib/tasks_scheduler/Src/*.c")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${APP_SOURCES}
        ${CMSIS_SOURCES}
        ${HAL_SOURCES}
        ${FREERTOS_SOURCES}
        ${DAC_X050X_SOURCES}
        ${SHVAL_SOURCES}
        ${TASKS_SCHEDULER_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Include
        lib/CMSIS/Include
        lib/CMSIS/Device/ST/STM32L0xx/Include
        lib/HAL/Include
        lib/FreeRTOS_Port/Include
        lib/FreeRTOS/Include
        lib/DAC_X050X/Include
        lib/shared_values/Include
        lib/tasks_scheduler/Include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        STM32L031xx
        DAC_80502
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
