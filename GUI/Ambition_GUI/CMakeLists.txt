cmake_minimum_required(VERSION 3.10)
project(RoverGuiStandalone)

# Use C++17 as specified in your PRD
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find System Libraries
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Define ImGui directory
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Create the executable and compile ImGui directly into it
add_executable(rover_ui 
    src/main.cpp
    src/panels/TopPanel.cpp
    src/panels/SidePanel.cpp
    src/views/View.cpp
    src/views/MaintenanceView.cpp
    src/views/NavigationView.cpp
    src/views/ProbingView.cpp
    src/views/ScienceView.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        src/views/ScienceView.h
)

# Tell the compiler where to find the ImGui header files (.h)
target_include_directories(rover_ui PRIVATE 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src/enums
)

# Link SDL2 and OpenGL
target_link_libraries(rover_ui PRIVATE ${SDL2_LIBRARIES} OpenGL::GL)

set(ROVER_UI_FONTS
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/fonts/Roboto-Medium.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/fonts/fa-solid-900.otf"
)
set_property(TARGET rover_ui APPEND PROPERTY LINK_DEPENDS "${ROVER_UI_FONTS}")
add_custom_command(TARGET rover_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:rover_ui>/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ROVER_UI_FONTS} "$<TARGET_FILE_DIR:rover_ui>/fonts"
    VERBATIM
)
