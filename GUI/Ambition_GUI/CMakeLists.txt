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
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Tell the compiler where to find the ImGui header files (.h)
target_include_directories(rover_ui PRIVATE 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
)

# Link SDL2 and OpenGL
target_link_libraries(rover_ui PRIVATE ${SDL2_LIBRARIES} OpenGL::GL)