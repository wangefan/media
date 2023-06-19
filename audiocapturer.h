#ifndef AUDIO_CAPTURER_H
#define AUDIO_CAPTURER_H
#include "worker.h"
#include <functional>

class AudioCapturer : public Worker {
public:
  AudioCapturer();
  virtual ~AudioCapturer();

  void Work() override;

  RET_CODE Init(std::unique_ptr<Properties> properties);
  void AddCallback(std::function<void(uint8_t *pcm, int32_t size)> callBack) {
    pcm_callback_ = callBack;
  };

private:
  int openPcmFile(const char *file_name);
  int closePcmFile();

private:
  std::function<void(uint8_t *pcm, int32_t size)> pcm_callback_;
  std::unique_ptr<Properties> properties_;
  int64_t pcm_start_time_;
  int64_t pcm_dst_time_;
  FILE *pcm_fp_;
  u_int16_t pcm_per_frame_size_;
  uint8_t *pcm_per_frame_;
  double frame_duration_;
};

#endif // AUDIO_CAPTURER_H
