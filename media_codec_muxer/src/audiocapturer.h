#ifndef AUDIO_CAPTURER_H
#define AUDIO_CAPTURER_H
#include "worker.h"
#include <functional>

namespace auc {

enum SampleFormat {
  PCM16Bit,
};

class AudioCapturer : public Worker {
public:
  AudioCapturer() = default;
  virtual ~AudioCapturer();

  bool Init(uint32_t sample_rate, uint32_t channel_count, uint32_t bit_rate);
  void AddCallback(
      std::function<void(uint8_t *pcm, int32_t size, int64_t time_stamp)>
          pcm_callback) {
    pcm_callback_ = pcm_callback;
  };
  void Work() override;

private:
  uint32_t sample_rate_;
  uint32_t sample_format_;
  uint32_t channel_count_;
  std::function<void(uint8_t *pcm, int32_t size, int64_t time_stamp)>
      pcm_callback_;
};

} // namespace auc
#endif // AUDIO_CAPTURER_H