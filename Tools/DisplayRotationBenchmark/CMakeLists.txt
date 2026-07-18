add_executable(display_rotation_benchmark
    DisplayRotationBenchmark.c
    ../../Apps/BrewieApp/src/Platform/Display_rotation.c
)

target_include_directories(display_rotation_benchmark PRIVATE
    ../../Apps/BrewieApp/src
)

target_link_libraries(display_rotation_benchmark PRIVATE
    lvgl
)

target_compile_options(display_rotation_benchmark PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
