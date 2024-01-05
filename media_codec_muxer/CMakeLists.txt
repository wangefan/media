cmake_minimum_required(VERSION 3.14)

project(media-codec-muxer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(${CMAKE_PROJECT_NAME}
  src/main.cpp
  src/dlog.h
  src/dlog.cpp
  src/mediaformat.h
  src/mediaformat.cpp
  src/mediamuxer.h
  src/mediamuxer.cpp
  src/mediaworker.h
  src/mediaworker.cpp
  src/worker.h
  src/worker.cpp
  src/rawdatabufferinfo.h
  src/encodeddatabufferinfo.h
  src/audiocapturer.h
  src/audiocapturer.cpp
  src/encoder.h
  src/encoder.cpp
  src/audioencoder.h
  src/audioencoder.cpp
  src/timesutil.h
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
)
target_link_libraries (${PROJECT_NAME} pthread PkgConfig::LIBAV)