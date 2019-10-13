cmake_minimum_required(VERSION 3.15)

project(raycaster CXX)

# Parse out the exact target platform
if(WIN32)
    set(TARGET_WINDOWS 1)
elseif(APPLE)
    if(IOS)
        set(TARGET_MACOS 1)
    else()
        set(TARGET_IOS 1)
    endif()
elseif(ANDROID)
    set(TARGET_ANDROID 1)
elseif(UNIX)
    set(TARGET_LINUX 1)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB source_common "src/*.cpp")
file(GLOB headers_common "src/*.hpp")
file(GLOB headers_common_c "src/*.h")
file(GLOB source_windows "src/windows/*.cpp")
file(GLOB source_linux "src/linux/*.cpp")

set(source ${source_common} ${source_windows} ${source_linux} ${headers_common} ${headers_common_c})
source_group("Source Files\\Windows" FILES ${source_windows})
source_group("Source Files\\Linux" FILES ${source_linux})

add_executable(raycaster ${source})
target_include_directories(raycaster PRIVATE "src")

set_target_properties(raycaster PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

# Apply platform specific libraries
if(${TARGET_LINUX})
    find_package(X11 REQUIRED)

    target_link_libraries(raycaster PRIVATE xcb xcb-image)
endif()