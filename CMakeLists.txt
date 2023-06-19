cmake_minimum_required(VERSION 3.5)

project(rtsp-publish LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rtsp-publish
    main.cpp
    worker.h worker.cpp
    audiocapturer.h audiocapturer.cpp
    pushworker.h pushworker.cpp
    utility.h utility.cpp
    dlog.h dlog.cpp
    mediabase.h
    timesutil.h
)
#target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME} pthread) 

install(TARGETS rtsp-publish
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
