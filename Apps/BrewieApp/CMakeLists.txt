if(CMAKE_CROSSCOMPILING)
    configure_file(${CMAKE_SOURCE_DIR}/lv_conf_target.h ${CMAKE_SOURCE_DIR}/lv_conf.h COPYONLY)
    add_executable(brewie_app
        src/main.c
        src/UI/UI.c
    )
else()
    find_package(SDL2 REQUIRED)
    add_executable(brewie_app
        src/main_sim.c
        src/UI/UI.c
    )
    target_link_libraries(brewie_app PRIVATE SDL2::SDL2)
endif()

target_include_directories(brewie_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
    ${CMAKE_SOURCE_DIR}/Shared/Protocol/include
    ${CMAKE_SOURCE_DIR}/Shared/Platform/include
)

if(TARGET lvgl)
    target_link_libraries(brewie_app PRIVATE lvgl)
endif()
