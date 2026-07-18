set(BREWIE_APP_COMMON_SOURCES
    src/App.c
    src/Comms/Comms.c
    src/Comms/Comms_link.c
    src/Comms/Protocol.c
    src/Comms/Transport_serial.c
    src/Logic/App_orchestrator.c
    src/Logic/Fault_logic.c
    src/Logic/Machine_state.c
    src/Logic/Machine_targets.c
    src/Logic/Startup_logic.c
    src/Logic/Status_view_model.c
    src/Logic/User_actions.c
    src/Platform/Display.c
    src/Platform/Display_rotation.c
    src/Platform/Logging.c
    src/Platform/Platform.c
    src/Platform/Time_base.c
    src/UI/Screen_status.c
    src/UI/Screen_fault.c
    src/UI/Screen_home.c
    src/UI/Screen_recipes.c
    src/UI/UI.c
    src/UI/UI_dialog.c
    src/UI/UI_status_bar.c
    src/UI/UI_theme.c
)

if(CMAKE_CROSSCOMPILING)
    # Target builds use the real SOM entry point and the DRM-enabled LVGL config selected
    # by the top-level CMakeLists.txt before LVGL is added. The display backend choice is
    # deliberately a compile-time define while the target display stack is still being
    # tested, so the known-good DRM binary and the experimental fbdev binary can be built
    # in separate directories without runtime ambiguity.
    add_executable(brewie_app
        src/main.c
        ${BREWIE_APP_COMMON_SOURCES}
    )

    target_include_directories(brewie_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_compile_definitions(brewie_app PRIVATE
        BREWIE_TARGET_DISPLAY_BACKEND_${BREWIE_TARGET_DISPLAY_BACKEND}=1
    )
else()
    # Simulator builds use a local entry point and SDL. They should exercise the same app,
    # logic, UI, and comms source files where possible, while avoiding target-only Linux DRM
    # headers and devices.
    find_package(SDL2 REQUIRED)

    add_executable(brewie_app
        src/main_sim.c
        ${BREWIE_APP_COMMON_SOURCES}
    )

    target_include_directories(brewie_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(brewie_app PRIVATE
        lvgl
        SDL2::SDL2
    )
endif()

if(CMAKE_CROSSCOMPILING)
    target_link_libraries(brewie_app PRIVATE
        lvgl
    )
endif()
