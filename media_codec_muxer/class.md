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

## UML Diagram

The updated diagram now shows `Encoder` as a direct child of `Worker`, unifying the two previous parallel hierarchies.

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
