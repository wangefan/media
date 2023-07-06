cmake_minimum_required(VERSION 3.5)

project(simple_playback LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
    libswresample
    libswscale
    libavdevice
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(simple_playback
    main.cpp
)

target_link_libraries (${PROJECT_NAME} pthread PkgConfig::LIBAV) 
target_link_libraries(${PROJECT_NAME}  ${SDL2_LIBRARIES})
install(TARGETS simple_playback
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
