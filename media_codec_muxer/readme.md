# C++ A/V Capture, Encoder & Muxer

## Overview

A multithreaded C++ framework that captures, encodes (H.264/AAC), and muxes audio/video streams into an MP4 file using the FFmpeg libraries. It is designed with a decoupled producer-consumer architecture for efficiency.

## Features

* **Capture**: Grabs desktop video (X11) and microphone audio (ALSA) via FFmpeg CLI.
* **Encode**: Uses `libavcodec` for H.264 video and AAC audio encoding.
* **Mux**: Uses `libavformat` to mux encoded streams into an MP4 container.
* **Architecture**: Thread-safe producer-consumer design.
* **Synchronization**: High-precision microsecond timestamps to ensure A/V sync.

## Core Components

* **`Worker`**: Base class for all threaded components.
* **`Capturers`**: Acquire raw audio/video data.
* **`Encoders`**: Encode the raw data into compressed formats.
* **`MediaMuxer`**: Writes encoded streams into the final media file.

## Dependencies

* CMake (>= 3.14)
* C++14 Compiler
* FFmpeg libraries (`libavcodec`, `libavformat`, `libavutil`)
* `pkg-config`

## How to Build

1.  **Install Dependencies** (Example for Debian/Ubuntu)
    ```bash
    sudo apt-get install build-essential cmake pkg-config libavformat-dev libavcodec-dev libavutil-dev
    ```

2.  **Compile the Project**
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```

3.  **Run**
    ```bash
    ./media-codec-muxer
    ```
