cmake_minimum_required(VERSION 3.18)

project(FreeBrewie_SOM_Stack C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Build LVGL examples" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "Build LVGL demos" FORCE)

add_subdirectory(external/lvgl)

if(CMAKE_CROSSCOMPILING)
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