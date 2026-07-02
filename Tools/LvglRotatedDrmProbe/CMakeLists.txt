add_executable(lvgl_rotated_drm_probe LvglRotatedDrmProbe.c)
target_compile_options(lvgl_rotated_drm_probe PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(lvgl_rotated_drm_probe PRIVATE /usr/include/libdrm)
target_link_libraries(lvgl_rotated_drm_probe PRIVATE lvgl drm)
