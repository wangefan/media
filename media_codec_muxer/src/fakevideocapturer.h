#ifndef FAKE_VIDEO_CAPTURER_H
#define FAKE_VIDEO_CAPTURER_H
#include "rawdatabufferinfo.h"
#include "worker.h"
#include <functional>

class FakeVideoCapturer : public Worker {
public:
  FakeVideoCapturer() = default;
  virtual ~FakeVideoCapturer();

  bool Init(uint32_t video_width, uint32_t video_height, uint32_t video_fps);
  void AddCallback(std::function<void(RawDataBufferInfo &)> yuv_callback) {
    yuv_callback_ = yuv_callback;
  };
  void Work() override;

private:
  uint32_t video_width_;
  uint32_t video_height_;
  uint32_t video_fps_;
  std::function<void(RawDataBufferInfo &)> yuv_callback_;
};

#endif // FAKE_VIDEO_CAPTURER_H