cmake_minimum_required(VERSION 3.18)

project(FreeBrewie_SOM_Stack C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(FREEBREWIE_BUILD_BREWIE_APP "Build the main Brewie app" ON)
option(FREEBREWIE_BUILD_CALIBRATION_APP "Build the calibration app" ON)
option(FREEBREWIE_BUILD_LAUNCHER "Build the launcher app" ON)
option(FREEBREWIE_BUILD_MCU_PROBE "Build the MCU probe tool" ON)

if(EXISTS ${CMAKE_SOURCE_DIR}/external/lvgl/CMakeLists.txt)
    add_subdirectory(external/lvgl)
else()
    message(WARNING "external/lvgl is not populated in this scaffold zip yet")
endif()

if(FREEBREWIE_BUILD_BREWIE_APP)
    add_subdirectory(Apps/BrewieApp)
endif()

if(FREEBREWIE_BUILD_CALIBRATION_APP)
    add_subdirectory(Apps/CalibrationApp)
endif()

if(FREEBREWIE_BUILD_LAUNCHER)
    add_subdirectory(Apps/Launcher)
endif()

if(FREEBREWIE_BUILD_MCU_PROBE)
    add_subdirectory(Tools/McuProbe)
endif()
