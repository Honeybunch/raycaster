cmake_minimum_required(VERSION 3.15)

project(raycaster CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB source_common "src/*.cpp")
file(GLOB source_windows "src/windows/*.cpp")
file(GLOB source_linux "src/linux/*.cpp")

set(source ${source_common} ${source_windows} ${source_linux})

add_executable(raycaster ${source})
target_include_directories(raycaster PRIVATE "src")