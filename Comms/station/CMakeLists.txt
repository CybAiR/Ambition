# ---- Station-specific dependencies ----
pkg_check_modules(OPENCV REQUIRED opencv4)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# ---- Station executable ----
add_executable(komsiki_station main.cpp)

target_include_directories(komsiki_station PRIVATE
    ${OPENCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(komsiki_station PRIVATE
    imgui_lib
    ${OPENCV_LIBRARIES}
    ${GST_LIBRARIES}
    OpenGL::GL
    pthread
    dl
)

target_compile_options(komsiki_station PRIVATE ${OPENCV_CFLAGS_OTHER} ${GST_CFLAGS_OTHER})
