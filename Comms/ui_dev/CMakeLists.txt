# ---- UI dev executable ----
add_executable(komsiki_ui_dev main.cpp)

target_link_libraries(komsiki_ui_dev PRIVATE
    imgui_lib
    OpenGL::GL
)

if(UNIX)
    target_link_libraries(komsiki_ui_dev PRIVATE pthread dl)
endif()
