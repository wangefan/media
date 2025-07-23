# Worker Class Hierarchy (Unified)

This document outlines the class hierarchy based on a unified architecture where all asynchronous components inherit from `Worker`.

## Inheriting Classes

In this unified design, all asynchronous components—both **Capturers** and **Encoders**—inherit from `Worker` to gain a common threading capability.

### Capturers
* `DesktopCapturer`
* `MicAudioCapturer`
* `FakeAudioCapturer`
* `FakeVideoCapturer`

### Encoders
* `Encoder` (as an abstract base for all encoders)
* `AudioEncoder` (inherits from `Encoder`)
* `VideoEncoder` (inherits from `Encoder`)

## Design Rationale

This unified design, where both capturers and encoders are `Worker`s, provides several key benefits:

* **Code Reusability**: Eliminates redundant thread management code that previously existed in the `Encoder` class.
* **Unified Interface**: All asynchronous components share the `Start()`, `Stop()`, and `Work()` methods, creating a consistent and predictable architecture.
* **Simplified Control**: Allows the main application to manage all capturers and encoders polymorphically, simplifying overall control flow.
## Concept
### Producer-Consumer
```mermaid
flowchart LR
    %% Define styles for clarity - changed actor color to purple
    classDef actor fill:#8e44ad,color:white,stroke-width:2px,stroke:#9b59b6
    classDef buffer fill:#ecf0f1,color:black,stroke-width:2px,stroke:#bdc3c7
    classDef codec fill:#8e44ad,color:white,stroke-width:2px,stroke:#9b59b6

    %% Define Nodes - updated Producer text
    Producer["<b>Producer</b><br/>DesktopCapturer &<br/>MediaWorker"]:::actor
    InputQueue["<b>Input Buffers</b><br/>(raw_data_queue_)<hr/>[YUV] [YUV] [...]"]:::buffer
    Encoder["<b>Consumer / Producer</b><hr/>VideoEncoder <br/>consumes raw buffers<br/>produces encoded packets"]:::codec
    OutputQueue["<b>Output Buffers</b><br/>(via EncodedCallback)<hr/>[H.264] [H.264] [...]"]:::buffer
    Muxer["<b>Consumer</b><br/>MediaMuxer"]:::actor

    %% Define Flow - updated edge labels
    Producer -- "DesktopCapturer produces raw buffer" --> InputQueue
    InputQueue -- "VideoEncoder consumes raw buffer" --> Encoder
    Encoder -- "VideoEncoder produces encoded packet" --> OutputQueue
    OutputQueue -- "MediaMuxer consumes packet" --> Muxer
```

## UML Diagram
The updated diagram now shows `Encoder` as a direct child of `Worker`, unifying the two previous parallel hierarchies.

### Classes
```mermaid
classDiagram
    direction LR

    class Worker {
        <<Abstract>>
        +Start() bool
        +Stop() void
        +Work()* void
    }

    class Encoder {
        <<Abstract>>
        #AVCodecContext* codec_ctx_
        #AVFrame* av_frame_
        +Init(MediaFormat)* bool
        #consume_queue(queue)* void
        #GetClassName()* const char*
    }

    class DesktopCapturer {
        +Work() void
    }

    class MicAudioCapturer {
        +Work() void
    }

    class FakeAudioCapturer {
        +Work() void
    }

    class FakeVideoCapturer {
        +Work() void
    }

    class AudioEncoder {
        +Init(MediaFormat) bool
        -consume_queue(queue) void
        -GetClassName() const char*
    }

    class VideoEncoder {
        +Init(MediaFormat) bool
        -consume_queue(queue) void
        -GetClassName() const char*
    }

    Worker <|-- DesktopCapturer
    Worker <|-- MicAudioCapturer
    Worker <|-- FakeAudioCapturer
    Worker <|-- FakeVideoCapturer

    Worker <|-- Encoder
    Encoder <|-- AudioEncoder
    Encoder <|-- VideoEncoder

```

### Sequence:
Video pipe:
```mermaid
sequenceDiagram
    participant main as Main
    participant mw as MediaWorker
    participant dc as DesktopCapturer Thread
    participant ve as VideoEncoder Thread
    participant mm as MediaMuxer

    main->>mw: create & Init()
    main->>mw: Start()
    mw->>dc: video_capturer_->Start()
    activate dc
    dc-->>mw: yuv_callback_(RawData)
    deactivate dc

    mw->>ve: QueueDataToEncode(RawData)
    activate ve
    ve-->>mw: encoded_callback_(H.264 Packet)
    deactivate ve
    
    mw->>mm: WriteSampleData(H.264 Packet)
```

Audio pipe:
```mermaid
sequenceDiagram
    participant main as Main
    participant mw as MediaWorker
    participant ac as MicAudioCapturer Thread
    participant ae as AudioEncoder Thread
    participant mm as MediaMuxer

    main->>mw: create & Init()
    main->>mw: Start()
    mw->>ac: audio_capturer_->Start()
    activate ac
    ac-->>mw: pcm_callback_(RawData)
    deactivate ac

    mw->>ae: QueueDataToEncode(RawData)
    activate ae
    ae-->>mw: encoded_callback_(AAC Packet)
    deactivate ae
    
    mw->>mm: WriteSampleData(AAC Packet)
```
