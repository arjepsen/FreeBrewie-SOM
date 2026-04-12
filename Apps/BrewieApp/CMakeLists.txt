set(BREWIE_APP_COMMON_SOURCES
    src/App/App.c
    src/UI/UI.c
    ${CMAKE_SOURCE_DIR}/Shared/Platform/src/Serial.c
    ${CMAKE_SOURCE_DIR}/Shared/Platform/src/Time.c
    ${CMAKE_SOURCE_DIR}/Shared/Protocol/src/Protocol.c
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
        src/UI/UI.c
    )

    target_link_libraries(brewie_app PRIVATE SDL2::SDL2)
endif()

target_include_directories(brewie_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/App
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
    ${CMAKE_SOURCE_DIR}/Shared/Platform/include
    ${CMAKE_SOURCE_DIR}/Shared/Protocol/include
)

target_link_libraries(brewie_app PRIVATE lvgl)
