cmake_minimum_required(VERSION 3.5)

project(rtsp-publish LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rtsp-publish
    main.cpp
    worker.h worker.cpp
    audiocapturer.h audiocapturer.cpp
    aacencoder.h aacencoder.cpp
    resampler.h resampler.cpp
    pushworker.h pushworker.cpp
    utility.h utility.cpp
    dlog.h dlog.cpp
    mediabase.h
    timesutil.h
    avpublishtime.h avpublishtime.cpp
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavcodec
    libavutil
)
#target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME} pthread PkgConfig::LIBAV)

install(TARGETS rtsp-publish
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
