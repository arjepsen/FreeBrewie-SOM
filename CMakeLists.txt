cmake_minimum_required(VERSION 3.18)

project(FreeBrewie_SOM_Stack C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Build LVGL examples" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "Build LVGL demos" FORCE)

set(BREWIE_TARGET_DISPLAY_BACKEND "drm" CACHE STRING "Target display backend: drm or fbdev")
set_property(CACHE BREWIE_TARGET_DISPLAY_BACKEND PROPERTY STRINGS drm fbdev)
if(NOT BREWIE_TARGET_DISPLAY_BACKEND STREQUAL "drm" AND
   NOT BREWIE_TARGET_DISPLAY_BACKEND STREQUAL "fbdev")
    message(FATAL_ERROR "BREWIE_TARGET_DISPLAY_BACKEND must be 'drm' or 'fbdev'")
endif()

# LVGL reads its configuration while external/lvgl is being added. Keep the simulator and
# target configs as separate files so configuring one build directory cannot silently change
# the behavior of the other. This matters because the target uses Linux DRM, while the local
# simulator should use SDL and must not include DRM headers.
if(CMAKE_CROSSCOMPILING)
    set(LV_BUILD_CONF_PATH
        "${CMAKE_SOURCE_DIR}/lv_conf_target.h"
        CACHE PATH "LVGL configuration file for the Brewie SOM target" FORCE
    )
else()
    set(LV_BUILD_CONF_PATH
        "${CMAKE_SOURCE_DIR}/lv_conf_sim.h"
        CACHE PATH "LVGL configuration file for the local simulator" FORCE
    )
endif()

add_subdirectory(external/lvgl)

if(CMAKE_CROSSCOMPILING)
    message(STATUS "Brewie target display backend: ${BREWIE_TARGET_DISPLAY_BACKEND}")

    # The target build links LVGL's Linux DRM backend against the armhf DRM headers and
    # library from the Bullseye VM/sysroot setup. These paths are target-only; adding them
    # to the simulator build is what previously caused local SDL builds to see drm.h.
    target_include_directories(lvgl PUBLIC
        /usr/include
        /usr/include/libdrm
    )

    target_link_directories(lvgl PUBLIC
        /usr/lib/arm-linux-gnueabihf
    )

    target_link_libraries(lvgl PUBLIC
        drm
    )
endif()

add_subdirectory(Apps/BrewieApp)
