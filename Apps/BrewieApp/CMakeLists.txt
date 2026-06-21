set(BREWIE_APP_COMMON_SOURCES
    src/App.c
    src/Comms/Comms.c
    src/Comms/Comms_link.c
    src/Comms/Protocol.c
    src/Comms/Transport_serial.c
    src/Logic/App_logic.c
    src/Logic/Fault_logic.c
    src/Logic/Machine_state.c
    src/Logic/Machine_targets.c
    src/Logic/Startup_logic.c
    src/Logic/User_actions.c
    src/Platform/Display.c
    src/Platform/Logging.c
    src/Platform/Platform.c
    src/Platform/Time_base.c
    src/UI/Screen_boot.c
    src/UI/Screen_fault.c
    src/UI/Screen_home.c
    src/UI/UI.c
    src/UI/UI_dialog.c
    src/UI/UI_status_bar.c
    src/UI/UI_theme.c
)

if(CMAKE_CROSSCOMPILING)
    configure_file(${CMAKE_SOURCE_DIR}/lv_conf_target.h ${CMAKE_SOURCE_DIR}/lv_conf.h COPYONLY)

    add_executable(brewie_app
        src/main.c
        ${BREWIE_APP_COMMON_SOURCES}
    )
else()
    find_package(SDL2 REQUIRED)

    add_executable(brewie_app
        src/main_sim.c
        ${BREWIE_APP_COMMON_SOURCES}
    )

    target_link_libraries(brewie_app PRIVATE SDL2::SDL2)
endif()

target_include_directories(brewie_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(brewie_app PRIVATE lvgl)