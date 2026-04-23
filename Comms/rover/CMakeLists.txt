cmake_minimum_required(VERSION 3.20)
project(komsiki_rover LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(komsiki_rover main.cpp)

target_include_directories(komsiki_rover PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(komsiki_rover PRIVATE ${GST_LIBRARIES} pthread)
target_compile_options(komsiki_rover PRIVATE ${GST_CFLAGS_OTHER})
